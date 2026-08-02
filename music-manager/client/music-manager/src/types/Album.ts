export interface Album {
  id: string;

  title: string;
  artist: string;

  cover?: string;

  trackCount: number;

  state?: "ready" | "duplicate" | "needs-metadata";
}
