// Patch helper for generated Max patchers: boxes, lines and the inlet and outlet counts of
// every object used, so tests can check each line. A trimmed copy of the one in
// devices/slush-wave/prototype/.../build-patch.mjs, shared by the breeder devices.

// Inlet and outlet counts for every object used. Max recomputes them on load; these only
// need to match what Max 8 builds. Add an object here only after checking the Max reference.
export function shape(text) {
  const [name, ...args] = text.split(" ");
  const sig = (n) => Array(n).fill("signal");
  const any = (n) => Array(n).fill("");
  switch (name) {
    case "midiin": return [1, ["int"]];
    case "midiparse": return [1, ["", "", "", "int", "int", "", "int"]];
    case "unpack": return [1, args.map((a) => (a.includes(".") ? "float" : "int"))];
    case "sel": return [2, ["bang", ""]];
    case "t": return [1, args.map((a) => ({ b: "bang", i: "int", f: "float" })[a] || "")];
    case "/": return [2, ["float"]];
    case "js": return [1, any(1)];
    case "route": return [2, any(args.length + 1)];
    case "prepend": return [1, any(1)];
    case "live.thisdevice": return [1, ["bang", "int", "int"]];
    case "deferlow": return [1, any(1)];
    case "line~": return [2, ["signal", "bang"]];
    case "cycle~": return [2, sig(1)];
    case "*~": case "+~": return [2, sig(1)];
    case "noise~": return [1, sig(1)];
    case "svf~": return [3, sig(4)];
    case "tanh~": return [1, sig(1)];
    case "lores~": return [3, sig(1)];
    case "clip~": return [3, sig(1)];
    case "plugout~": return [2, []];
    default: throw new Error("No shape for object: " + text);
  }
}

// Every object name shape() knows: the standard Max 8 and MSP objects a device may use.
export const MAX_OBJECTS = new Set([
  "midiin", "midiparse", "route", "unpack", "sel", "t", "/", "js", "prepend", "live.thisdevice",
  "deferlow", "line~", "cycle~", "*~", "+~", "noise~", "svf~", "tanh~", "lores~", "clip~", "plugout~",
]);

export class Patch {
  constructor() {
    this.boxes = [];
    this.lines = [];
    this.count = 0;
    this.x = 20;
    this.y = 20;
  }

  at(x, y) {
    this.x = x;
    this.y = y;
    return this;
  }

  place(width, height = 22) {
    const rect = [this.x, this.y, width, height];
    this.y += height + 8;
    return rect;
  }

  add(box) {
    const id = `obj-${++this.count}`;
    this.boxes.push({ box: { id, ...box } });
    return id;
  }

  obj(text, extra = {}) {
    const [numinlets, outlettype] = shape(text);
    return this.add({
      maxclass: "newobj",
      numinlets,
      numoutlets: outlettype.length,
      outlettype,
      patching_rect: this.place(Math.max(40, text.length * 6 + 10)),
      text,
      ...extra,
    });
  }

  msg(text = "") {
    return this.add({
      maxclass: "message",
      numinlets: 2,
      numoutlets: 1,
      outlettype: [""],
      patching_rect: this.place(Math.max(60, text.length * 6 + 10)),
      text,
    });
  }

  comment(text) {
    return this.add({
      maxclass: "comment",
      numinlets: 1,
      numoutlets: 0,
      patching_rect: this.place(Math.min(420, text.length * 6 + 10)),
      text,
    });
  }

  connect(from, outlet, to, inlet) {
    this.lines.push({ patchline: { destination: [to, inlet], source: [from, outlet] } });
  }
}
