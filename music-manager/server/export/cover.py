# downsize and write the album cover

from pathlib import Path
import base64
import io

from PIL import Image

from models import Album

COVER_SIZE = (64, 64)

# create a 64x64 BMP cover image
# returns the output path or None if no cover exists
def write_cover(
        album: Album,
        output_dir: Path,
) -> Path | None:
    if not album.cover_data:
        return None

    try:
        image_data = album.cover_data

        # remove data URL prefix
        if "," in image_data:
            image_data = image_data.split(",", 1)[1]

        # perform conversion operations
        raw = base64.b64decode(image_data)
        img = Image.open(io.BytesIO(raw))
        img = img.convert("RGB")
        img = img.resize(COVER_SIZE, Image.LANCZOS)

        # save and return
        output_path = output_dir / "cover.bmp"
        img.save(output_path, format="BMP")
        return output_path

    except Exception as e:
        print(f"[cover] failed for {album.title}: {e}")

        return None
