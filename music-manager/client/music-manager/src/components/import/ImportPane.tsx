import "./ImportPane.css";

import { useState } from "react";

import AlbumGrid from "../AlbumGrid";
import { Album } from "../../types/Album";
import AlbumEditorDialog from "../AlbumEditor/AlbumEditorDialog";

import { scanFolders } from "../../api/scanner";
import { startExport, getExportProgress, downloadExport } from "../../api/exporter";

function ImportPane() {
  const [albums, setAlbums] = useState<Album[]>([]);
  const [errors, setErrors] = useState<string[]>([]);

  const [exporting, setExporting] = useState(false);
  const [progress, setProgress] = useState(0);

  const [selectedAlbum, setSelectedAlbum] = useState<Album | null>(null);

  async function handleAddAlbums() {
    const path = prompt("album folder");
    
    if (!path) return;

    const result = await scanFolders([path]);
    console.log(result);
    setAlbums(result.albums);
    setErrors(result.errors);
  }

  async function handleExport() {
    if (albums.length === 0) {
      alert("No albums to export.");
      return;
    }

    setExporting(true);

    await startExport(
      albums,
      {
	format: "mp3",
	bitrate: 320,
      }
    );

    const interval = setInterval(async () => {
      const status = await getExportProgress();

      setProgress(status.progress);

      if (!status.running) {
	clearInterval(interval);
	if (status.status === "complete") {
	  downloadExport();
	}

	setExporting(false);
      }
    }, 500);
  }
  
  return (
    <div className="ImportPane">
      <header className="pane-header">
	<h2>import</h2>
	<div className="header-buttons">
	  <button className="primary-button"
		  onClick={handleAddAlbums}>+ add albums</button>
	  <button className="export-button"
		  onClick={handleExport}
		  disabled={exporting}>
	    {exporting
	    ? `exporting ${Math.round(progress)}%`
	    : "export to SD card"}
	  </button>
	</div>

      </header>

      <div className="export-settings">
	<label>
	  format
	  <select defaultValue="mp3">
            <option value="mp3">MP3</option>
          </select>
	</label>

	<label>
	  bitrate
	  <select defaultValue="320">
	    <option value="320">320 kbps</option>
	    <option value="256">256 kbps</option>
	    <option value="192">192 kbps</option>
	  </select>
	</label>
      </div>

      <div className="album-area">
	<AlbumGrid
	  albums={albums}
	  onAlbumClick={setSelectedAlbum}
	  emptyMessage="no albums added yet." />

	{selectedAlbum && (
	  <AlbumEditorDialog
	    album={selectedAlbum}
	    onClose={() => setSelectedAlbum(null)}
	    onSave={(updatedAlbum) => {
	      setAlbums(current =>
		current.map(album =>
		  album.id === updatedAlbum.id ? updatedAlbum : album));
	      
	      setSelectedAlbum(null);
	    }} />
	)}
      </div>
    </div>
  );
}

export default ImportPane;

