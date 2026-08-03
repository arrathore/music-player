export interface Album {
  id: string;

  title: string;
  artist: string;

  cover_data?: string;

  trackCount: number;

  state?: "ready" | "duplicate" | "needs-metadata";
}
