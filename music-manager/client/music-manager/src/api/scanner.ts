import { Album } from "../types/Album"

interface ScanResponse {
  albums: Album[];
  errors: string[];
}

export async function scanFolders(
  paths: string[]
): Promise<ScanResponse> {

  const response = await fetch("/api/scan-folders", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ paths }),
  });

  if (!response.ok) {
    throw new Error("Scan failed.");
  }

  return response.json();
}
