import "./ImportPane.css";

import AlbumGrid from "../AlbumGrid";
import { Album } from "../../types/Album";


function ImportPane() {
  const albums: Album[] = [];
  
  return (
    <div className="ImportPane">
      <header className="pane-header">
	<h2>import</h2>
	<button className="primary-button">+ add albums</button>
      </header>

      <div className="import-settings">
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
	  emptyMessage="no albums added yet." />
      </div>
    </div>
  );
}

export default ImportPane;

