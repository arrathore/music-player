import "./AlbumEditorDialog.css";

import { useEffect, useState } from "react";

import { Album } from "../../types/Album";

interface AlbumEditorDialogProps {
  album: Album;
  onSave: (album: Album) => void;
  onClose: () => void;
}

function AlbumEditorDialog({ album, onSave, onClose }: AlbumEditorDialogProps) {
  const [editedAlbum, setEditedAlbum] = useState<Album>(album);

  useEffect(() => {
    // make a deep copy whenever a new album is opened
    setEditedAlbum({
      ...album,
      tracks: album.tracks.map(track => ({ ...track })),
    });
  }, [album]);

  function updateField<K extends keyof Album>(
    field: K,
    value: Album[K]
  ) {
    setEditedAlbum(prev => ({ ...prev, [field]: value }));
  }

  function handleSave() {
    onSave(editedAlbum);
  }

  return (
        <div
      className="dialog-backdrop"
      onClick={onClose}
    >

      <div
        className="AlbumEditorDialog"
        onClick={e => e.stopPropagation()}
      >

        <header className="dialog-header">
          <h2>edit album</h2>

          <button
            className="close-button"
            onClick={onClose}
            type="button"
          >
            ✕
          </button>
        </header>

        <div className="dialog-body">

          <div className="cover-column">

            {editedAlbum.cover_data ? (
              <img
                className="editor-cover"
                src={editedAlbum.cover_data}
                alt={`${editedAlbum.title} cover`}
              />
            ) : (
              <div className="editor-cover-placeholder">
                no cover
              </div>
            )}

            <p className="cover-source">
              source: {editedAlbum.cover_source}
            </p>

          </div>

          <div className="metadata-column">

            <label>
              title
              <input
                type="text"
                value={editedAlbum.title}
                onChange={e =>
                  updateField("title", e.target.value)
                }
              />
            </label>

            <label>
              artist
              <input
                type="text"
                value={editedAlbum.artist}
                onChange={e =>
                  updateField("artist", e.target.value)
                }
              />
            </label>

            <label>
              genre
              <input
                type="text"
                value={editedAlbum.genre}
                onChange={e =>
                  updateField("genre", e.target.value)
                }
              />
            </label>

            <label>
              year
              <input
                type="text"
                value={editedAlbum.year}
                onChange={e =>
                  updateField("year", e.target.value)
                }
              />
            </label>

          </div>

        </div>

        <footer className="dialog-footer">

          <button
            type="button"
            onClick={onClose}
          >
            cancel
          </button>

          <button
            className="primary-button"
            type="button"
            onClick={handleSave}
          >
            save
          </button>

        </footer>

      </div>

    </div>
  );
}

export default AlbumEditorDialog;
