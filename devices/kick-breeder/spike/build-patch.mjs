// Generates the Kick Breeder spike device, so the patcher JSON stays consistent and reviewable.
//
//   node build-patch.mjs   (or npm run build)   writes kick-breeder.js (joined from kick-voice.js
//                                                and shared/breeder/), "Kick Breeder Spike.maxpat",
//                                                and packs "Kick Breeder Spike.amxd" as an instrument
//
// The panel, keys, slots and saved kicks are shared with the other breeder devices
// (shared/breeder/build.mjs). This file has only what makes it a kick: the voice section.

import { fileURLToPath } from "node:url";
import { breederDevice } from "../../../shared/breeder/build.mjs";

export { SAVED, PLAY_KEYS, ACTION_KEYS, SLOTS } from "../../../shared/breeder/build.mjs";

export const FILE = "Kick Breeder Spike";
export const JS_FILE = "kick-breeder.js";
// Engine parameters, in PARAMS order (kick-voice.js): short names for the locks.
export const PARAM_NAMES = [
  "Pitch", "Sweep", "Sweep T", "Curve", "Decay", "Click", "Click T",
  "Noise", "Noise D", "Noise T", "Comp", "Sat", "Tone",
];
export const PARTS = ["statics", "freq", "noise", "amp", "click"];

// One monophonic kick: a swept sine body, a band-passed noise click and a low-passed noise
// layer, each times its envelope, then saturation, tone and gain.
function voice(p) {
  p.at(240, 1000);
  p.comment("Voice. Body: cycle~ swept by the pitch envelope, times the amp envelope.");
  const freqLine = p.obj("line~");
  const osc = p.obj("cycle~");
  const ampLine = p.obj("line~");
  const body = p.obj("*~");
  p.connect(freqLine, 0, osc, 0);
  p.connect(osc, 0, body, 0);
  p.connect(ampLine, 0, body, 1);

  p.at(520, 1000);
  p.comment("Click: band-passed noise. Noise layer: low-passed noise. Each times its envelope.");
  const noise = p.obj("noise~");
  const clickFilter = p.obj("svf~ 3000 0.3");
  const clickLine = p.obj("line~");
  const click = p.obj("*~");
  const noiseFilter = p.obj("svf~ 2000 0.");
  const noiseLine = p.obj("line~");
  const noiseLayer = p.obj("*~");
  p.connect(noise, 0, clickFilter, 0);
  p.connect(clickFilter, 2, click, 0);
  p.connect(clickLine, 0, click, 1);
  p.connect(noise, 0, noiseFilter, 0);
  p.connect(noiseFilter, 0, noiseLayer, 0);
  p.connect(noiseLine, 0, noiseLayer, 1);

  p.at(760, 1000);
  p.comment("Settings: click tone, noise tone, saturation in, saturation out, tone, gain.");
  const statics = p.obj("unpack 0. 0. 0. 0. 0. 0.");

  p.at(240, 1250);
  p.comment("Mix, saturation: tanh~(x * k) / tanh(k), tone: lores~, level-matched gain, velocity, then clip~ for safety.");
  const mix = p.obj("+~");
  const mixNoise = p.obj("+~");
  const satIn = p.obj("*~ 1.");
  const shaper = p.obj("tanh~");
  const satOut = p.obj("*~ 1.");
  const tone = p.obj("lores~ 8000 0.");
  const gain = p.obj("*~ 0.5");
  const velocity = p.obj("*~ 1.");
  const safety = p.obj("clip~ -1. 1.");
  const out = p.obj("plugout~");
  p.connect(body, 0, mix, 0);
  p.connect(click, 0, mix, 1);
  p.connect(mix, 0, mixNoise, 0);
  p.connect(noiseLayer, 0, mixNoise, 1);
  p.connect(mixNoise, 0, satIn, 0);
  p.connect(satIn, 0, shaper, 0);
  p.connect(shaper, 0, satOut, 0);
  p.connect(satOut, 0, tone, 0);
  p.connect(tone, 0, gain, 0);
  p.connect(gain, 0, velocity, 0);
  p.connect(velocity, 0, safety, 0);
  p.connect(safety, 0, out, 0);
  p.connect(safety, 0, out, 1);
  p.connect(statics, 0, clickFilter, 1);
  p.connect(statics, 1, noiseFilter, 1);
  p.connect(statics, 2, satIn, 1);
  p.connect(statics, 3, satOut, 1);
  p.connect(statics, 4, tone, 1);
  p.connect(statics, 5, gain, 1);
  const vel = p.obj("/ 127.");
  p.connect(vel, 0, velocity, 1);
  // cycle~ is a cosine; phase 0.75 starts it at zero, so the hit does not click.
  const phase = p.msg("0.75");
  p.connect(phase, 0, osc, 1);

  return {
    inputs: { statics, freq: freqLine, noise: noiseLine, amp: ampLine, click: clickLine },
    resets: [phase],
    velocity: vel,
  };
}

const device = breederDevice({
  title: "Kick Breeder",
  noun: "kick",
  file: FILE,
  jsFile: JS_FILE,
  voiceFile: "kick-voice.js",
  paramNames: PARAM_NAMES,
  parts: PARTS,
  width: 754,
  voice,
}, new URL("./", import.meta.url));

export const { MESSAGES, patcherText, jsText } = device;

if (process.argv[1] === fileURLToPath(import.meta.url)) device.write();
