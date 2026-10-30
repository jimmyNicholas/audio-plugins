// Packs a Max patcher (.maxpat JSON) into an unfrozen Max for Live device (.amxd), and back.
//
// An .amxd is a series of chunks: a 4-byte ASCII tag, a 4-byte little-endian length, then
// the data. "ampf" holds the device type, "meta" four bytes we keep at zero, and "ptch" the
// patcher JSON followed by a NUL byte. Format read from Ableton's maxdevtools
// (maxdiff/amxd_textconv.py); see devices/tap/docs/RESEARCH.md.
//
// The patcher also records its device type, as project.amxdtype (the ampf tag read as a
// big-endian integer). Packing checks the two agree, so a patcher saved as one kind of
// device is never packed as another; see devices/slush-wave/docs/research/amxd-audio-effect.md.
//
//   node tools/amxd.mjs pack [--type midi|audio] <in.maxpat> <out.amxd>   (default midi)
//   node tools/amxd.mjs unpack <in.amxd> <out.maxpat>

import fs from "node:fs";
import { fileURLToPath } from "node:url";

export const MIDI_EFFECT = "mmmm";
export const AUDIO_EFFECT = "aaaa";

const TYPES = { midi: MIDI_EFFECT, audio: AUDIO_EFFECT };

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
  const json = JSON.parse(patcherText); // fail early on broken JSON
  const expected = Buffer.from(deviceType, "ascii").readUInt32BE(0);
  const amxdtype = json.patcher && json.patcher.project && json.patcher.project.amxdtype;
  if (amxdtype === undefined) {
    throw new Error("Patcher has no project.amxdtype; expected " + expected + " for " + deviceType);
  }
  if (amxdtype !== expected) {
    throw new Error("Patcher project.amxdtype is " + amxdtype + ", but " + deviceType + " needs " + expected);
  }
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
  const args = process.argv.slice(2);
  let type = "midi";
  const flag = args.indexOf("--type");
  if (flag !== -1) type = args.splice(flag, 2)[1];
  const [command, input, output] = args;
  if (command === "pack" && input && output && TYPES[type]) {
    fs.writeFileSync(output, pack(fs.readFileSync(input, "utf8"), TYPES[type]));
  } else if (command === "unpack" && input && output) {
    fs.writeFileSync(output, unpack(fs.readFileSync(input)));
  } else {
    console.error("Usage: node amxd.mjs pack [--type midi|audio] <in.maxpat> <out.amxd> | unpack <in.amxd> <out.maxpat>");
    process.exit(1);
  }
}
