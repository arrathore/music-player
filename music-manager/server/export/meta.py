# write meta.txt files

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
