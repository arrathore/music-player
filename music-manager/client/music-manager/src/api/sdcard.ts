export async function scanSDCard(path: string) {
  const response = await fetch("/api/sd-card/scan", {
    method: 'POST',
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({path}),
  });

  if (!response.ok) {
    throw new Error("Failed to scan SD card.");
  }

  return response.json();
}
