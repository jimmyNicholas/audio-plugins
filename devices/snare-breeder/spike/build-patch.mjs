// Generates the Snare Breeder spike device, so the patcher JSON stays consistent and reviewable.
//
//   node build-patch.mjs   (or npm run build)   writes snare-breeder.js (joined from snare-voice.js
//                                                and shared/breeder/), "Snare Breeder Spike.maxpat",
//                                                and packs "Snare Breeder Spike.amxd" as an instrument
//
// The panel, keys, slots and saved snares are shared with the other breeder devices
// (shared/breeder/build.mjs). This file has only what makes it a snare: the voice section.

import { fileURLToPath } from "node:url";
import { breederDevice } from "../../../shared/breeder/build.mjs";

export { SAVED, PLAY_KEYS, ACTION_KEYS, SLOTS } from "../../../shared/breeder/build.mjs";

export const FILE = "Snare Breeder Spike";
export const JS_FILE = "snare-breeder.js";
// Engine parameters, in PARAMS order (snare-voice.js): short names for the locks.
export const PARAM_NAMES = [
  "Pitch", "Sweep", "Sweep T", "Ratio", "Ring", "Body D", "Balance", "Wires D",
  "Wires T", "Width", "Bursts", "Spread", "Snap", "Comp", "Sat", "Tone",
];
export const PARTS = ["statics", "freq", "wires", "amp", "snap"];

// One monophonic snare. Body: two cycle~, the second at Ratio times the first and Ring loud,
// both swept by one pitch envelope, times the body envelope. Wires: band-passed noise times
// the wires envelope. Snap: high-passed noise times its envelope. Then saturation, tone, gain.
function voice(p) {
  p.at(240, 1000);
  p.comment("Voice. Body: two cycle~ swept by the pitch envelope (the second at Ratio, Ring loud), times the body envelope.");
  const freqLine = p.obj("line~");
  const osc = p.obj("cycle~");
  const ratio = p.obj("*~ 1.6");
  const overtone = p.obj("cycle~");
  const ring = p.obj("*~ 0.5");
  const modes = p.obj("+~");
  const ampLine = p.obj("line~");
  const body = p.obj("*~");
  p.connect(freqLine, 0, osc, 0);
  p.connect(freqLine, 0, ratio, 0);
  p.connect(ratio, 0, overtone, 0);
  p.connect(overtone, 0, ring, 0);
  p.connect(osc, 0, modes, 0);
  p.connect(ring, 0, modes, 1);
  p.connect(modes, 0, body, 0);
  p.connect(ampLine, 0, body, 1);

  p.at(560, 1000);
  p.comment("Wires: band-passed noise. Snap: high-passed noise. Each times its envelope.");
  const noise = p.obj("noise~");
  const wiresFilter = p.obj("svf~ 3000 0.4");
  const wiresLine = p.obj("line~");
  const wires = p.obj("*~");
  const snapFilter = p.obj("svf~ 2500 0.2");
  const snapLine = p.obj("line~");
  const snap = p.obj("*~");
  p.connect(noise, 0, wiresFilter, 0);
  p.connect(wiresFilter, 2, wires, 0);
  p.connect(wiresLine, 0, wires, 1);
  p.connect(noise, 0, snapFilter, 0);
  p.connect(snapFilter, 1, snap, 0);
  p.connect(snapLine, 0, snap, 1);

  p.at(820, 1000);
  p.comment("Settings: ratio, ring, wires tone, wires resonance, saturation in, saturation out, tone, gain.");
  const statics = p.obj("unpack 0. 0. 0. 0. 0. 0. 0. 0.");

  p.at(240, 1300);
  p.comment("Mix, saturation: tanh~(x * k) / tanh(k), tone: lores~, level-matched gain, velocity, then clip~ for safety.");
  const mix = p.obj("+~");
  const mixSnap = p.obj("+~");
  const satIn = p.obj("*~ 1.");
  const shaper = p.obj("tanh~");
  const satOut = p.obj("*~ 1.");
  const tone = p.obj("lores~ 8000 0.");
  const gain = p.obj("*~ 0.5");
  const velocity = p.obj("*~ 1.");
  const safety = p.obj("clip~ -1. 1.");
  const out = p.obj("plugout~");
  p.connect(body, 0, mix, 0);
  p.connect(wires, 0, mix, 1);
  p.connect(mix, 0, mixSnap, 0);
  p.connect(snap, 0, mixSnap, 1);
  p.connect(mixSnap, 0, satIn, 0);
  p.connect(satIn, 0, shaper, 0);
  p.connect(shaper, 0, satOut, 0);
  p.connect(satOut, 0, tone, 0);
  p.connect(tone, 0, gain, 0);
  p.connect(gain, 0, velocity, 0);
  p.connect(velocity, 0, safety, 0);
  p.connect(safety, 0, out, 0);
  p.connect(safety, 0, out, 1);
  p.connect(statics, 0, ratio, 1);
  p.connect(statics, 1, ring, 1);
  p.connect(statics, 2, wiresFilter, 1);
  p.connect(statics, 3, wiresFilter, 2);
  p.connect(statics, 4, satIn, 1);
  p.connect(statics, 5, satOut, 1);
  p.connect(statics, 6, tone, 1);
  p.connect(statics, 7, gain, 1);
  const vel = p.obj("/ 127.");
  p.connect(vel, 0, velocity, 1);
  // cycle~ is a cosine; phase 0.75 starts both at zero, so the hit does not click.
  const phase = p.msg("0.75");
  p.connect(phase, 0, osc, 1);
  p.connect(phase, 0, overtone, 1);

  return {
    inputs: { statics, freq: freqLine, wires: wiresLine, amp: ampLine, snap: snapLine },
    resets: [phase],
    velocity: vel,
  };
}

const device = breederDevice({
  title: "Snare Breeder",
  noun: "snare",
  file: FILE,
  jsFile: JS_FILE,
  voiceFile: "snare-voice.js",
  paramNames: PARAM_NAMES,
  parts: PARTS,
  width: 754,
  voice,
}, new URL("./", import.meta.url));

export const { MESSAGES, patcherText, jsText } = device;

if (process.argv[1] === fileURLToPath(import.meta.url)) device.write();
