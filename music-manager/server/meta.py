# manage meta.txt files

from pathlib import Path
from models import Album

# write meta.txt for an album
def write_meta(
        album: Album,
        output_dir: Path,
        tracks: list[str]
):
    meta_path = output_dir / "meta.txt"

    with open(meta_path, "w", encoding="utf-8") as f:
        f.write("[meta]\n")
        f.write(f"Title: {album.title}\n")
        f.write(f"Artist: {album.artist}\n")
        f.write(f"Genre: {album.genre}\n")
        f.write(f"Year: {album.year}\n")

        f.write("\n")

        f.write("[tracks]\n")
        for track in tracks:
            f.write(f"{track}\n")

# read a meta.txt file
# returns title, artist, genre, year, tracks
def read_meta(path: Path) -> dict | None:
    if not path.exists():
        return None

    try:
        data = {
            "title": "",
            "artist": "",
            "genre": "",
            "year": "",
            "tracks": [],
        }

        section = None

        with open(path, encoding="utf-8") as f:
            for raw_line in f:
                line = raw_line.strip()

                # determine section
                if not line:
                    continue
                if line == "[meta]":
                    section = "meta"
                    continue
                if line == "[tracks]":
                    section = "tracks"
                    continue

                if section == "meta":
                    if ":" not in line: # skip
                        continue

                    key, value = line.split(":", 1)
                    key = key.strip().lower()
                    value = value.strip()

                    if key == "title":
                        data["title"] = value
                    elif key == "artist":
                        data["artist"] = value
                    elif key == "genre":
                        data["genre"] = value
                    elif key == "year":
                        data["year"] = value

                elif section == "tracks":
                    data["tracks"].append(line)

        return data

    except Exception as e:
        print(f"[meta] failed to read {path}: {e}")
        return None

                        
