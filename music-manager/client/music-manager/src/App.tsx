import React from 'react';
import './App.css';

import TitleBar from "./components/title-bar/TitleBar";
import ImportPane from "./components/import/ImportPane";
import SDCardPane from "./components/sdcard/SDCardPane";

function App() {
  return (
    <div className="App">
      <TitleBar />
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
