import './SDCardPane.css';

import { useState } from "react";

import AlbumGrid from "../AlbumGrid";

import { Album } from "../../types/Album";
import { SDCardInfo } from "../../types/SDCardInfo";

import { scanSDCard } from "../../api/sdcard";

function formatBytes(bytes: number): string {
  const units = ["B", "KB", "MB", "GB", "TB"];

  let value = bytes;
  let unit = 0;

  while (value >= 1024 && unit < units.length - 1) {
    value /= 1024;
    unit++;
  }

  return `${value.toFixed(1)} ${units[unit]}`;
}

function SDCardPane() {
  const [albums, setAlbums] = useState<Album[]>([]);

  const [drivePath, setDrivePath] = useState("");
  const [drive, setDrive] = useState<SDCardInfo | null>(null);

  // get the user's SD card
  async function handleSelectSDCard() {
    const path = prompt("SD card path");

    if (!path) return;

    const result = await scanSDCard(path);
    
    setDrivePath(path);
    setAlbums(result.albums);
    setDrive(result.drive);
  }
  
  return (
    <div className="SDCardPane">
      <header className="pane-header">
	<h2>SD card contents</h2>
      </header>
      
      <div className="sdcard-controls">
	<button
	  className="primary-button"
	  onClick={handleSelectSDCard} >
	  select SD card...
	  </button>

	<p className="drive-path">
	  {drivePath || "no SD card selected."}
	</p>
	
	<p className="storage-info">
	  {drive &&
	    `${formatBytes(drive.total_bytes - drive.free_bytes)} used / ${formatBytes(drive.total_bytes)}`}
	</p>
      </div>

      <div className="album-area">
	<AlbumGrid
	  albums={albums}
	  emptyMessage="no albums on SD card." />
      </div>
    </div>
  );
}

export default SDCardPane;


