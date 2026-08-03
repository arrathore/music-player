from pathlib import Path
import subprocess
import shutil

from models import Track

# convert a track into the requested format
# returns the path in the exported file
def transcode_track(
        track: Track,
        output_dir: Path,
        options,
) -> Path:
    output_format = options.format
    bitrate = options.bitrate

    filename = make_output_filename(track, output_format)

    output_path = output_dir / filename

    # if already correct format, copy instead
    if track.format == output_format:
        shutil.copy2(track.path, output_path)

        return output_path

    cmd = [
        "ffmpeg", "-y", "-i", track.path,
    ]

    # metadata
    cmd += [
        "-metadata", f"title={track.title}",
    ]

    if track.track_number:
        cmd += [
            "-metadata",
            f"track={track.track_number}",
        ]

    # bitrate
    if output_format == "mp3":
        cmd += [
            "-b:a",
            f"{bitrate}k",
        ]

    cmd.append(
        str(output_path)
    )

    result = subprocess.run(
        cmd,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
    )

    if result.returncode != 0:
        raise RuntimeError(
            f"ffmpeg failed for {track.path}\n"
            f"{result.stderr.decode()}"
        )

    return output_path

def make_output_filename(
        track: Track,
        extension: str,
) -> str:
    # avoid filesystem issues
    safe_title = (
        track.title
        .replace("/", "_")
        .replace("\\", "_")
    )

    if track.track_number:
        return (
            f"{track.track_number:02d} "
            f"{safe_title}.{extension}"
        )

    return f"{safe_title}.{extension}"
