# classes used throughout the application

from pydantic import BaseModel
from typing import Optional

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
    cover_data: Optional[str] # base64 encoded thumbnail, None if no cover (complete data URL)
    cover_source: str         # "embedded" | "file" | "none"
    tracks: list[Track]
    track_count: int
