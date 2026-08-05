# manage album covers

from pathlib import Path
import base64
import io

from PIL import Image

from models import Album

COVER_SIZE = (64, 64)

'''
HELPERS:
'''
# decode a Base64 url into raw bytes
def decode_data_url(data_url: str) -> bytes:
    if "," in data_url:
        data_url = data_url.split(",", 1)[1]

    return base64.b64decode(data_url)

# encode raw bytes into a Base64 url
def encode_data_url(data: bytes, mime: str) -> str:
    encoded = base64.b64encode(data).decode("utf-8")
    return f"data:{mime};base64,{encoded}"



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
        raw = decode_data_url(album.cover_data)
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

# read a cover.bmp and return a data URL
def read_cover(path: Path) -> str | None:
    if not path.exists():
        return None

    try:
        return encode_data_url(path.read_bytes(), "image/bmp")

    except Exception as e:
        print(f"[cover] failed to read {path}: {e}")
        return None

    
