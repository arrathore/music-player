from pathlib import Path
import psutil
import shutil
import uuid

from pydantic import BaseModel

from models import Album, Track
from meta import read_meta
from cover import read_cover

'''
MODELS
'''

# information about an SD card
class SDCardInfo(BaseModel):
    path: str
    name: str
    total_bytes: int
    free_bytes: int

# info and albums in the SD card
class SDCardContents(BaseModel):
    drive: SDCardInfo
    albums: list[Album]


# read one exported album
def load_album(folder: Path) -> Album | None:
    meta = read_meta(folder / "meta.txt")

    if meta is None:
        return None

    tracks: list[Track] = []

    for index, filename in enumerate(meta["tracks"], start=1):
        path = folder / filename

        tracks.append(Track(
            filename = filename,
            path = str(path),
            title = Path(filename).stem,
            track_number = index,
            duration = 0,
            format=path.suffix.lstrip(".").lower(),
        ))

    cover = read_cover(folder / "cover.bmp")

    return Album(
        id = str(uuid.uuid4()),
        source_path = str(folder),
        title = meta["title"],
        artist = meta["artist"],
        year = meta["year"],
        genre = meta["genre"],
        cover_data = cover,
        cover_source = "file" if cover else "none",
        tracks = tracks,
        track_count = len(tracks),
    )

# scan an SD card for exported albums    
def scan_sd_card(path: str) -> SDCardContents | None:
    root = Path(path)

    if not root.exists():
        return None

    usage = shutil.disk_usage(root)

    albums: list[Album] = []

    # every dir containing meta.txt could be an album
    for directory in root.rglob("*"):
        if not directory.is_dir():
            continue

        meta_file = directory / "meta.txt"

        if not meta_file.exists():
            continue

        album = load_album(directory)

        if album:
            albums.append(album)

    return SDCardContents(
        drive=SDCardInfo(
            path = str(root),
            name = root.name,
            total_bytes = usage.total,
            free_bytes = usage.free,
        ),
        albums = albums,
    )

        
    

