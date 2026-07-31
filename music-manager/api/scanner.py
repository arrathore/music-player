import os
import uuid
import base64
from pathlib import Path
from typing import Optional

from fastapi import APIRouter, HTTPException
from pydantic import BaseModel
from mutagen import File as MutagenFile
from mutagen.id3 import ID3NoHeaderError
from PIL import Image
import io

router = APIRouter()

"""
MODELS
"""

class Track(BaseModel):
    filename: str             # original filename on disk
    path: str                 # full absolute path
    title: str
    track_number: int         # for sorting, 0 if unknown
    duration: int             # seconds, 0 if unknown
    format: str               # mp3, flac, wav, m4a, ogg

class Album(BaseModel):
    id: str                   # uuid generated on scan
    source_path: str          # orignal folder path
    title: str
    artist: str
    year: str
    genre: str
    cover_data: Optional[str] # base64 encoded thumbnail, None if no cover
    cover_source: str         # "embedded" | "file" | "none"
    tracks: list[Track]

class ScanRequest(BaseModel):
    paths: list[str]

class ScanResponse(BaseModel):
    albums: list[Album]
    errors: list[str]

class UpdateAlbumRequest(BaseModel):
    album: Album

"""
CONSTANTS
"""

AUDIO_EXTENSIONS = {".mp3", ".m4a", ".flac", ".wav", ".ogg", ".aac"}

# common cover art filenames
COVER_FILENAMES = [
    "cover.jpg", "cover.jpeg", "cover.png",
    "folder.jpg", "folder.jpeg", "folder.png",
    "front.jpg",  "front.jpeg",  "front.png",
    "album.jpg",  "album.jpeg",  "album.png",
    "albumart.jpg", "albumart.jpeg",
]

# thumbnail size for display in UI
THUMBNAIL_SIZE = (64, 64)

"""
HELPERS
"""

# convert PIL image to base64 JPEG str
def encode_image(img: Image.Image) -> str:
    img = img.convert("RGB")
    img.thumbnail(THUMBNAIL_SIZE, Image.LANCZOS)
    buf = io.BytesIO()
    img.save(buf, format="JPEG", quality=85)
    return base64.b64encode(buf.getvalue()).decode("utf-8")

# extract embedded cover art from mutagen file object
# return base64 JPEG str or None
def extract_embedded_cover(mutagenFile) -> Optional[str]:
    try:
        # ID3
        if hasattr(mutagenFile, "tags") and mutagenFile.tags:
            # APIC frame
            for key in mutagenFile.tags.keys():
                if key.startswith("APIC"):
                    apic = mutagenFile.tags[key]
                    img = Image.open(io.BytesIO(apic.data))
                    return encode_image(img)

            # MP4 cover (M4A)
            if "covr" in mutagenFile.tags:
                cover_data = mutagenFile.tags["covr"][0]
                img = Image.open(io.BytesIO(bytes(cover_data)))
                return encode_image(img)

            # FLAC / OGG picture block
            if hasattr(mutagenFile, "pictures") and mutagenFile.pictures:
                img = Image.open(io.BytesIO(mutagenFile.pictures[0].data))
                return encode_image(img)

    except Exception as e:
        print(f"[scanner] embedded cover extraction failed: {e}")

    return None

# look for a cover image file in the album folder
# returns base64 JPEG string or None
def find_folder_cover(folder: Path) -> Optional[str]:
    for name in COVER_FILENAMES:
        candidate = folder / name
        if candidate.exists():
            try:
                img = Image.open(candidate)
                return encode_image(img)
            except Exception as e:
                print(f"[scanner] folder cover read failed: {e}")
    return None

# read a tag value a mutagen file
def get_tag(mutagenFile, *keys: str, default: str = "") -> str:
    if not mutagenFile or not mutagenFile.tags:
        return default
    for f in keys:
        try:
            val = mutagenFile.tags.get(key)
            if val is None:
                continue
            # ID3 frames have a .text list
            if hasattr(val, "text") and val.text:
                return str(val.text[0]).strip()
            # MP4 / FLAC / OGG values are lists
            if isInstance(val, list) and val:
                return str(val[0]).strip()
            return str(val).strip()
        except Exception:
            continue
    return default

# extract track number as an integer
# handles 'n' and 'n/total' formats
def get_track_number(mutagenFile) -> int:
    raw = get_tag(mutagenFile,
                  "TRCK",        # ID3
                  "tracknumber", #FLAC / OGG
                  "trkn")        # MP4

    if not raw:
        return 0
    try:
        # handle 'n/total'
        return int(raw.split("/")[0])
    except ValueError:
        return 0

# get duration in seconds    
def get_duration(mutagenFile) -> int:
    try:
        if mutagenFile and mutagenFile.info:
            return int(mutagenFile.info.length)
    except Exception:
        pass
    return 0

# scan a single audio file and return a Track object or None on failure
def scan_track(path: Path) -> Optional[Track]:
    try:
        mf = MutagenFile(path, easy=False)

        title = get_tag(mf,
                        "TIT2",        # ID3
                        "title",       # FLAC / OGG
                        "\xa9nam"      # MP4
                        ) or path.stem # fall back to filename without extension

        return Track(
            filename = path.name,
            path = str(path),
            title = title,
            track_number = get_track_number(mf),
            duration = get_duration(mf),
            format = path.suffix.lstrip(".").lower(),
        )

    except Exception as e:
        print(f"[scanner] failed to scan track {path}: {e}")
        return None

# drive album-level metadata from a mutagen file
# returns (title, artist, year, genre)
def derive_album_meta(mutagenFile, folder: Path) -> tuple[str, str, str, str]:
    title = get_tag(mutagenFile,
                    "TALB",          # ID3
                    "album",         # FLAC / OGG
                    "\xa9alb",       # MP4
                    ) or folder.name # fall back to folder name

    artist = get_tag(mutagenFile,
                     "TPE1",         # ID3 lead artist
                     "TPE2",         # ID3 album artist
                     "albumartist",  # FLAC / OGG
                     "artist",
                     "\xa9ART",      # MP4
                     "aART")         # MP4 album artist
 
    year = get_tag(mutagenFile,
                   "TDRC",           # ID3 recording date
                   "TYER",           # ID3 year (older)
                   "date",           # FLAC / OGG
                   "\xa9day"         # MP4
                   )
    
    # trim to just the year if full date given
    if year and len(year) > 4:
        year = year[:4]
 
    genre = get_tag(mutagenFile,
                    "TCON",  # ID3
                    "genre", # FLAC / OGG
                    "\xa9gen" # MP4
                    )
 
    return title, artist, year, genre

# scan an album folder
# returns (Album, None) on success or (None, error) on failure
def scan_folder(folder_path: str) -> tuple[Optional[Album], Optional[str]]:
    folder = Path(folder_path)

    if not folder.exists():
        return None, f"Folder not found: {folder_path}"
    if not folder.is_dir():
        return None, f"Not a folder: {folder_path}"

    # find all audio files
    audio_files = sorted([
        f for f in folder.iterdir()
        if f.is_file() and f.suffix.lower() in AUDIO_EXTENSIONS
    ])

    if not audio_files:
        return None, f"No audio files found in: {folder_path}"

    # scan all tracks
    tracks = []
    for f in audio_files:
        track = scan_track(f)
        if track:
            tracks.append(track)

    if not tracks:
        return None, f"Failed to read any tracks in: {folder_path}"

    # sort by track number, falling back to filename order for ties / zeros
    track.sort(key=lambda t: (t.track_number if t.track_number > 0 else 9990, t.filename))

    # derive album metadata from the first successfully read file
    album_title = folder.name
    album_arist = ""
    album_year = ""
    album_genre = ""

    for f in audio_files:
        try:
            mf = MutagenFile(f, easy=False)
            if mf and mf.tags:
                album_title, album_artist, album_year, album_genre = derive_album_meta(mf, folder)
                break
        except Exception:
            continue

    # cover art
    cover_data = None
    cover_source = "none"

    for f in audio_files:
        try: # embedded
            mf = MutagenFile(f, easy=False)
            cover_data = extract_embedded_cover(mf)
            if cover_data:
                cover_source = "embedded"
                break
        except Exception:
            continue

        if not cover_data: # folder file
            cover_data = find_folder_cover(folder)
            if cover_data:
                cover_source = "file"

        return Album(
            id=str(uuid.uuid4()),
            source_path=str(folder),
            title=album_title,
            artist=album_artist,
            year=album_year,
            genre=album_genre,
            cover_data=cover_data,
            cover_source=cover_source,
            tracks=tracks,
        ), None

# scan one or more album folders and return Album objects    
@router.post("/scan-folders", response_model=ScanResponse)
def scan_folders(req: ScanRequest):
    if not req.paths:
        raise HTTPException(status_code=400, detail="No paths provided")

    albums = []
    errors = []

    for path in req.paths:
        album, error = scan_folder(path)
        if album:
            albums.append(album)
        if error:
            errors.append(error)

    return ScanResponse(albums=albums, errors=errors)

# accept a user-edited Album object and return it validated
@router.post("/update-album", response_model=Album)
def update_album(req: UpdateAlbumRequest):
    album = req.album

    # id check
    if not album.id:
        album.id = str(uuid.uuid64())

    # ensure track list is not empty
    if not album.tracks:
        raise HTTPException(status_code=400, detail="Album must have at least 1 track")

    return album
    
                    


