import React from 'react';
import './App.css';

import ImportPane from "./components/import/ImportPane";
import SDCardPane from "./components/sdcard/SDCardPane";

function App() {
  return (
    <div className="App">
      <div id="main">
	<div className="pane">
	  <ImportPane />
	</div>
	<div className= "pane">
	  <SDCardPane />
	</div>
      </div>
    </div>
  );
}

export default App;
