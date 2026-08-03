from fastapi import APIRouter, HTTPException
from fastapi.responses import FileResponse

from pydantic import BaseModel

from export import export_engine
from models import Track, Album

router = APIRouter(prefix="/export", tags=["export"])

class ExportOptions(BaseModel):
    format: str
    bitrate: int

class ExportRequest(BaseModel):
    albums: list[Album]
    options: ExportOptions

# start a new export job
@router.post("")
async def export_music(request: ExportRequest):
    if export_engine.is_running():
        raise HTTPException(
            status_code=409,
            detail="An export is already in progress."
        )

    export_engine.start_export(request)

    return {
        "status": "started"
    }

# return the current export progress
@router.get("/progress")
async def export_progress():
    return {
        "running": export_engine.is_running(),
        "progress": export_engine.get_progress(),
        "status": export_engine.get_status(),
    }

# cancel the current export
@router.post("/cancel")
async def cancel_export():
    if not export_engine.is_running():
        return {
            "status":"idle"
        }

    export_engine.cancel_export()

    return {
        "status": "cancelling"
    }

# download the completed export
@router.get("/download")
async def download_export():
    if export_engine.is_running():
        raise HTTPException(
            status_code=409,
            detail="Export still in progress."
        )

    output = export_engine.get_output_file()

    if output is None:
        raise HTTPException(
            status_code=404,
            detail="No completed export available."
        )

    return FileResponse(
        path=output,
        filename="music_export.zip",
        media_type="application/zip",
    )

