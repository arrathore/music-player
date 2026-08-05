# orchestrates conversion and export process

from pathlib import Path
import shutil
import tempfile
import threading
import zipfile

from export.transcoder import transcode_track
from meta import write_meta
from cover import write_cover

class ExportCancelled(Exception):
    pass

# current export state
_running = False
_progress = 0.0
_status = "idle"
_cancel_requested = False
_output_file: Path | None = None

_job_thread: threading.Thread | None = None

# start export in a background thread
def start_export(request):
    global _job_thread
    global _running
    global _cancel_requested
    global _progress
    global _status
    global _output_file

    if _running:
        raise RuntimeError("Export already running")

    _running = True
    _cancel_requested = False
    _progress = 0.0
    _status = "starting"
    _output_file = None

    _job_thread = threading.Thread(
        target=_run_export,
        args=(request,),
        daemon=True,
    )

    _job_thread.start()

# request cancellation    
def cancel_export():
    global _cancel_requested
    global _status

    _cancel_requested = True
    _status = "cancelling"

def is_running():
    return _running

def get_progress():
    return _progress

def get_status():
    return _status

def get_output_file():
    return _output_file

# main export workflow
def _run_export(request):
    global _running
    global _progress
    global _status
    global _output_file

    temp_dir = None

    try:
        _status = "exporting"

        temp_dir = Path(
            tempfile.mkdtemp(prefix="sd_music_export_")
        )

        total_tracks = sum(
            len(album.tracks)
            for album in request.albums
        )

        completed_tracks = 0

        for album in request.albums:
            if _cancel_requested:
                raise ExportCancelled()

            album_dir = (
                temp_dir / album.artist / album.title
            )

            album_dir.mkdir(parents=True, exist_ok=True)

            exported_tracks = []

            for track in album.tracks:

                if _cancel_requested:
                    raise ExportCancelled()

                output_track = transcode_track(
                    track, album, album_dir, request.options,
                )

                exported_tracks.append(output_track.name)

                completed_tracks += 1

                _progress = (
                    (completed_tracks / total_tracks) * 100
                )

            write_meta(album, album_dir, exported_tracks)
            write_cover(album, album_dir)

        _status = "packaging"

        output = Path(tempfile.gettempdir()) / "music_export.zip"

        with zipfile.ZipFile(
                output,
                "w",
                zipfile.ZIP_DEFLATED
        ) as z:

            for file in temp_dir.rglob("*"):
                if file.is_file():
                    z.write(
                        file,
                        file.relative_to(temp_dir)
                    )

        _output_file = output
        _progress = 100
        _status = "complete"

    except ExportCancelled:
        _status = "cancelled"

        if temp_dir:
            shutil.rmtree(temp_dir, ignore_errors=True)

    except Exception as e:
        print(f"[export] failed: {e}")
        _status = "error"

    finally:
        _running = False

        
        
