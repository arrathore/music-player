from fastapi import APIRouter, HTTPException
from pydantic import BaseModel

from sdcard import sdcard_engine

router = APIRouter(prefix="/sd-card", tags=["sd-card"])

class SDCardScanRequest(BaseModel):
    path: str

# return the albums stored on the selected SD card
@router.post("/scan")
async def get_contents(request: SDCardScanRequest):
    contents = sdcard_engine.scan_sd_card(request.path)

    if contents is None:
        raise HTTPException(status_code=404, detail="Drive not found.")

    return contents

