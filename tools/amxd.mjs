// Packs a Max patcher (.maxpat JSON) into an unfrozen Max for Live device (.amxd), and back.
//
// An .amxd is a series of chunks: a 4-byte ASCII tag, a 4-byte little-endian length, then
// the data. "ampf" holds the device type, "meta" four bytes we keep at zero, and "ptch" the
// patcher JSON followed by a NUL byte. Format read from Ableton's maxdevtools
// (maxdiff/amxd_textconv.py); see docs/m4l/RESEARCH.md.
//
//   node m4l/tools/amxd.mjs pack <in.maxpat> <out.amxd>
//   node m4l/tools/amxd.mjs unpack <in.amxd> <out.maxpat>

import fs from "node:fs";
import { fileURLToPath } from "node:url";

export const MIDI_EFFECT = "mmmm";

function chunk(tag, data) {
  const header = Buffer.alloc(8);
  header.write(tag, 0, "ascii");
  header.writeUInt32LE(data.length, 4);
  return Buffer.concat([header, data]);
}

export function readChunks(buffer) {
  const chunks = [];
  let offset = 0;
  while (offset < buffer.length) {
    if (offset + 8 > buffer.length) throw new Error("Truncated chunk header at byte " + offset);
    const tag = buffer.toString("ascii", offset, offset + 4);
    const length = buffer.readUInt32LE(offset + 4);
    const end = offset + 8 + length;
    if (end > buffer.length) throw new Error("Chunk " + tag + " runs past the end of the file");
    chunks.push({ tag, data: buffer.subarray(offset + 8, end) });
    offset = end;
  }
  return chunks;
}

export function pack(patcherText, deviceType = MIDI_EFFECT) {
  JSON.parse(patcherText); // fail early on broken JSON
  const text = patcherText.endsWith("\n") ? patcherText : patcherText + "\n";
  return Buffer.concat([
    chunk("ampf", Buffer.from(deviceType, "ascii")),
    chunk("meta", Buffer.alloc(4)),
    chunk("ptch", Buffer.concat([Buffer.from(text, "utf8"), Buffer.from([0])])),
  ]);
}

export function unpack(buffer) {
  const ptch = readChunks(buffer).find((c) => c.tag === "ptch");
  if (!ptch) throw new Error("No ptch chunk: not a Max for Live device?");
  if (ptch.data.subarray(0, 4).toString("ascii") === "mx@c") {
    throw new Error("Device is frozen. Unfreeze it in Max and save it before unpacking.");
  }
  let end = ptch.data.length;
  while (end > 0 && ptch.data[end - 1] === 0) end--;
  return ptch.data.subarray(0, end).toString("utf8");
}

if (process.argv[1] === fileURLToPath(import.meta.url)) {
  const [command, input, output] = process.argv.slice(2);
  if (command === "pack" && input && output) {
    fs.writeFileSync(output, pack(fs.readFileSync(input, "utf8")));
  } else if (command === "unpack" && input && output) {
    fs.writeFileSync(output, unpack(fs.readFileSync(input)));
  } else {
    console.error("Usage: node amxd.mjs pack <in.maxpat> <out.amxd> | unpack <in.amxd> <out.maxpat>");
    process.exit(1);
  }
}
