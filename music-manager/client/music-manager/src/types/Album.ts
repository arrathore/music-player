import { Track } from "../types/Track";

export interface Album {
  id: string;

  title: string;
  artist: string;
  year: string;
  genre: string;

  cover_data?: string;
  cover_source: string;

  tracks: Track[];
  track_count: number;

  state?: "ready" | "duplicate" | "needs-metadata";
}
