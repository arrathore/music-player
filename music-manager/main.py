import uvicorn
import webbrowser
import threading
from fastapi import FastAPI
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse

app = FastAPI(title="SD music manager")

# API routers
app.include_router(scanner_router, prefix="/api")
# app.include_router(sdcard_router, prefix="/api")
# app.include_router(exporter_router, prefix="/api")
# app.include_router(settings_router, prefix="/api")


# serve frontend
app.mount("/static", StaticFiles(directory="client"), name="static")

# index
@app.get("/")
def root():
    return FileResponse("client/index.html")

# open browser after server starts
def open_browser():
    webbrowser.open("http://localhost:8000")

if __name__ == "__main__":
    # open browser 1 second after server starts
    timer = threading.Timer(1.0, open_browser)
    timer.start()

    uvicorn.run("main:app", host="127.0.0.1", port=8000, reload=False)

