import './SDCardPane.css';

function SDCardPane() {
  return (
    <div className="SDCardPane">
      <header className="pane-header">
	<h2>sd card</h2>
	<button className="primary-button">export</button>
      </header>

      <div className="sdcard-controls">
	<label htmlFor="drive-select">drive</label>
	<select id="drive-select" defaultValue="">
	  <option value="">select a drive...</option>
	</select>

	<p className="storage-info">
	  no SD card selected.
	</p>
      </div>

      <div className="album-area">
	<p className="empty-message">
	  no SD card selected.
	</p>
      </div>
    </div>
  );
}

export default SDCardPane;


