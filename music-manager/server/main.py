from fastapi import FastAPI
import uvicorn

from scanner import router as scanner_router
# from sdcard import router as sdcard_router
from export.exporter import router as exporter_router
# from settings import router as settings_router

app = FastAPI(title="SD music manager")

# API routers
app.include_router(scanner_router, prefix="/api")
# app.include_router(sdcard_router, prefix="/api")
app.include_router(exporter_router, prefix="/api")
# app.include_router(settings_router, prefix="/api")

if __name__ == "__main__":
    uvicorn.run("main:app", host="127.0.0.1", port=8000, reload=False)

