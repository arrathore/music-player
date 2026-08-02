import "./AlbumGrid.css"

import AlbumCard from "./AlbumCard";
import { Album } from "../types/Album";

interface AlbumGridProps {
  albums: Album[];

  onAlbumClick?: (album: Album) => void;

  emptyMessage?: string;
}

function AlbumGrid({
  albums,
  onAlbumClick,
  emptyMessage = "no albums."
}: AlbumGridProps) {

  if (albums.length === 0) {
    return (
      <p className="empty-message">
	{emptyMessage}
      </p>
    );
  }

  return (
      <div className="AlbumGrid">
	{albums.map(album => (
	<AlbumCard
	  key={album.id}
	  album={album}
	  onClick={() => onAlbumClick?.(album)} />
	))}
      </div>
  );
}

export default AlbumGrid;
