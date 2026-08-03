import { Album } from "../types/Album";

interface AlbumCardProps {
  album: Album;
  onClick?: () => void;
}

function AlbumCard({ album, onClick }: AlbumCardProps) {
  return (
	  <button
	    className="AlbumCard"
	    onClick={onClick}
            type="button" >

	    
	    <div className="album-cover">
	      {album.cover_data ? (
	      <img
		src={album.cover_data}
		alt={`${album.title} cover`} />
	      ) : (
	      <div className="album-cover-placeholder">
		[no cover]
	      </div>
	      )}
	    </div>

	    <div className="album-info">
	      <h3>{album.title}</h3>

	      <p>{album.artist}</p>

	      <span>{album.trackCount} tracks</span>

	      {album.state === "duplicate" && (
	      <span className="album-warning">
		* Duplicate
	      </span>
	      )}
	    </div>
	    
	  </button>
  );
}

export default AlbumCard;
