export interface Album {
  id: string;

  title: string;
  artist: string;

  cover_data?: string;

  track_count: number;

  state?: "ready" | "duplicate" | "needs-metadata";
}
