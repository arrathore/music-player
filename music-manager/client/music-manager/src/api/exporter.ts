import { Album } from "../types/Album";

interface ExportOptions {
  format: string;
  bitrate: number;
}

interface ExportRequest {
  albums: Album[];
  options: ExportOptions;
}

export async function startExport(albums: Album[], options: ExportOptions) {
  const response = await fetch("/api/export", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      albums, options,
    } satisfies ExportRequest),
  });

  if (!response.ok) {
    throw new Error("Failed to start export");
  }

  return response.json()
}

export async function getExportProgress() {
  const response = await fetch("/api/export/progress");

  if (!response.ok) {
    throw new Error("Failed to get export progress");
  }

  return response.json();
}

export async function downloadExport() {
  //   window.location.href = "/api/export/download";
  const response = await fetch("/api/export/download");

  console.log(response.status);
  console.log(response.headers.get("content-type"));

  if (!response.ok) throw new Error("Download failed")

  const blob = await response.blob();
  const url = URL.createObjectURL(blob);

  const link = document.createElement("a");
  link.href = url;
  link.download = "music_export.zip";
  link.click();

  URL.revokeObjectURL(url);

}

