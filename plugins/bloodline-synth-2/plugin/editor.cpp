#include "editor.h"
#include <cmath>

using namespace bs2;
using namespace synth;

namespace {

const juce::Colour kBg(0xff15171c), kPanel(0xff1f2229), kTile(0xff292d36), kEdge(0xff3a3f4b);
const juce::Colour kFg(0xffe8e6e1), kMuted(0xff8d8f96);
const juce::Colour kAccent(0xffb48cff), kParent(0xffffc857), kKeep(0xff9be564), kLock(0xff7cc6fe);

const juce::Colour kGroupColour[kNumGroups] = {
  juce::Colour(0xff7cc6fe), juce::Colour(0xffff8c69), juce::Colour(0xff9be564), juce::Colour(0xffd59bf6), juce::Colour(0xfff4d35e),
};
// One colour a zone label: None, Bass, Keys, Lead, Pad, Pluck.
const juce::Colour kZoneColour[kNumZones] = {
  juce::Colour(0xffc9c9c9), juce::Colour(0xffff6b6b), juce::Colour(0xff7cc6fe), juce::Colour(0xfff4d35e), juce::Colour(0xffd59bf6),
  juce::Colour(0xff9be564),
};
const juce::Colour kTraitColour[kNumTraits] = {
  juce::Colour(0xff7cc6fe), juce::Colour(0xffd59bf6), juce::Colour(0xffff8c69), juce::Colour(0xfff4d35e),
  juce::Colour(0xffff6b6b), juce::Colour(0xff9be564), juce::Colour(0xff6ee7d8), juce::Colour(0xffffa94d),
};
const Zone kZones[kNumZones] = { Zone::None, Zone::Bass, Zone::Keys, Zone::Lead, Zone::Pad, Zone::Pluck };

juce::Font font(float size, bool bold = false) { return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain)); }
juce::Font mono(float size) { return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain)); }
juce::String letter(int i) { return juce::String::charToString((juce::juce_wchar) ('A' + i)); }
juce::String num(double v, int places) { return places == 0 ? juce::String(juce::roundToInt(v)) : juce::String(v, places); }
juce::String star() { return juce::String::fromUTF8("\xe2\x98\x85"); }
juce::String noteName(int pitch) { return juce::String(keyName(pitch % 12)) + juce::String(pitch / 12 - 1); }
juce::Colour zoneColour(Zone z) { return kZoneColour[juce::jlimit(0, kNumZones - 1, (int) z)]; }

int groupOf(int s) {
  for (int g = 0; g < kNumGroups; ++g)
    if (s >= group(g).first && s < group(g).first + group(g).count) return g;
  return 0;
}

juce::String shapeName(double v) {
  const char* names[] = { "sine", "tri", "saw", "square" };
  const int k = juce::jlimit(0, 3, (int) std::round(v * 3.0));
  return std::abs(v * 3.0 - k) < 0.15 ? juce::String(names[k])
                                      : juce::String(names[juce::jmin(2, (int) (v * 3.0))]) + ">" + names[juce::jmin(3, (int) (v * 3.0) + 1)];
}

juce::String formatValue(int s, double norm) {
  const double v = toReal(s, norm);
  const juce::String unit(info(s).unit);
  if (s == ShapeA || s == ShapeB) return shapeName(v);
  if (unit == "Hz") return v >= 1000 ? juce::String(v / 1000.0, 1) + " kHz" : num(v, v < 10 ? 2 : 0) + " Hz";
  if (unit == "s") return v < 1.0 ? num(v * 1000.0, v < 0.01 ? 1 : 0) + " ms" : juce::String(v, 2) + " s";
  if (unit == "st") return juce::String(v, 1) + " st";
  if (unit == "ct") return juce::String(juce::roundToInt(v)) + " ct";
  if (unit == "oct") return juce::String(v, 2) + " oct";
  if (unit == "bits") return juce::String(v, 1) + " bits";
  if (unit == "dB") return juce::String(v, 1) + " dB";
  return juce::String(juce::roundToInt(v * 100.0)) + "%";
}

double shapeAt(double phase, double shape) {
  const double seg = shape * 3.0;
  const int k = juce::jmin(2, (int) seg);
  const double f = seg - k;
  const double sine = std::sin(2 * juce::MathConstants<double>::pi * phase);
  const double tri = 1.0 - 4.0 * std::abs(phase - 0.5);
  const double saw = 2.0 * phase - 1.0;
  const double sq = phase < 0.5 ? 1.0 : -1.0;
  switch (k) {
    case 0: return sine + (tri - sine) * f;
    case 1: return tri + (saw - tri) * f;
    default: return saw + (sq - saw) * f;
  }
}

double snapSixteenth(double beat) { return std::round(beat / kSixteenth) * kSixteenth; }

constexpr int kWidth = 1280, kHeight = 780, kPanelW = 616, kRow1 = 46, kRow2 = 80, kTilesY = 116, kTileH = 152;
constexpr int kSettingsY = 278, kSettingsH = 330, kKeepY = 620, kKeepH = 146;
constexpr double kSpreadMin = 0.02, kSpreadMax = 0.5;

} // namespace

BloodlineSynth2Editor::BloodlineSynth2Editor(BloodlineSynth2Processor& p) : AudioProcessorEditor(&p), proc_(p), view_(p.view()) {
  setSize(kWidth, kHeight);
  startTimerHz(30);
}

BloodlineSynth2Editor::~BloodlineSynth2Editor() { stopTimer(); }

void BloodlineSynth2Editor::timerCallback() {
  view_ = proc_.view();
  repaint();
}

void BloodlineSynth2Editor::act(const std::function<void(Studio&)>& change) {
  proc_.edit(change);
  timerCallback();
}

void BloodlineSynth2Editor::button(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled,
                                   std::function<void()> click) {
  const auto f = r.toFloat().reduced(1.5f);
  const float a = enabled ? 1.0f : 0.35f;
  g.setColour((on ? kAccent : kTile).withMultipliedAlpha(a));
  g.fillRoundedRectangle(f, 4.0f);
  g.setColour((on ? kAccent : kEdge).withMultipliedAlpha(a));
  g.drawRoundedRectangle(f.reduced(0.5f), 4.0f, 1.0f);
  g.setColour((on ? kBg : kFg).withMultipliedAlpha(a));
  g.setFont(font(12.0f, on));
  g.drawFittedText(text, r.reduced(2, 0), juce::Justification::centred, 1, 0.7f);
  if (enabled) hits_.push_back({ r, std::move(click) });
}

void BloodlineSynth2Editor::slider(juce::Graphics& g, juce::Rectangle<int> r, double value, juce::Colour c, std::function<void(double)> drag,
                                   std::function<void()> release) {
  const auto bar = r.toFloat().withSizeKeepingCentre((float) r.getWidth(), 6.0f);
  value = juce::jlimit(0.0, 1.0, value);
  g.setColour(kTile);
  g.fillRoundedRectangle(bar, 3.0f);
  g.setColour(c);
  g.fillRoundedRectangle(bar.withWidth(juce::jmax(3.0f, bar.getWidth() * (float) value)), 3.0f);
  g.fillEllipse(bar.getX() + bar.getWidth() * (float) value - 5.0f, bar.getCentreY() - 5.0f, 10.0f, 10.0f);
  hits_.push_back({ r, {}, std::move(drag), std::move(release) });
}

void BloodlineSynth2Editor::label(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text) {
  g.setColour(kMuted);
  g.setFont(font(11.0f, true));
  g.drawText(text, r, juce::Justification::centredLeft);
}

juce::String BloodlineSynth2Editor::soundName(int pad) const {
  if (pad < 0 || pad >= Sounds::kPads || !view_.pads[(size_t) pad].full) return "no sound";
  return view_.pads[(size_t) pad].name;
}

void BloodlineSynth2Editor::paint(juce::Graphics& g) {
  hits_.clear();
  g.fillAll(kBg);
  g.setColour(kFg);
  g.setFont(font(18.0f, true));
  g.drawText("BLOODLINE SYNTH 2", 16, 10, 240, 28, juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(mono(11.0f));
  g.drawText("prototype  |  breed sounds on the left, keep them, then breed loops that play them on the right", 230, 10, 800, 28,
             juce::Justification::centredLeft);
  g.setColour(kEdge);
  g.fillRect(16 + kPanelW + 7, kRow1, 2, kHeight - kRow1 - 12);
  paintSounds(g, { 16, 0, kPanelW, kHeight });
  paintLoops(g, { 16 + kPanelW + 16, 0, kPanelW, kHeight });
  paintCarry(g);
}

// ---------------------------------------------------------------------------------------------
// Left: the sounds.

void BloodlineSynth2Editor::paintSounds(juce::Graphics& g, juce::Rectangle<int> area) {
  const int x = area.getX(), w = area.getWidth();
  // [zone][back][new]
  label(g, { x, kRow1, 44, 26 }, "ZONE");
  for (int z = 0; z < kNumZones; ++z)
    button(g, { x + 44 + z * 62, kRow1, 62, 26 }, zoneName(kZones[z]), view_.zone == kZones[z], true,
           [this, z] { act([z](Studio& s) { s.sounds.setZone(kZones[z]); }); });
  button(g, { x + w - 140, kRow1, 64, 26 }, "Back", false, view_.soundBack, [this] { act([](Studio& s) { s.sounds.back(); }); });
  button(g, { x + w - 72, kRow1, 72, 26 }, "New", false, true, [this] { act([](Studio& s) { s.sounds.fresh(); }); });

  // [riff][spread]
  label(g, { x, kRow2, 44, 26 }, "RIFF");
  for (int m = 0; m < BloodlineSynth2Processor::kRiffModes; ++m)
    button(g, { x + 44 + m * 60, kRow2, 60, 26 }, BloodlineSynth2Processor::riffName(m), view_.riff == m, true,
           [this, m] { proc_.setRiff(m); timerCallback(); });
  {
    auto r = juce::Rectangle<int>(x + 352, kRow2, w - 352, 26);
    label(g, r.removeFromLeft(52), "SPREAD");
    g.setColour(kFg);
    g.setFont(mono(11.0f));
    g.drawText(juce::String(view_.soundSpread, 2), r.removeFromRight(36), juce::Justification::centredRight);
    r.removeFromRight(8);
    slider(g, r.reduced(4, 0), (view_.soundSpread - kSpreadMin) / (kSpreadMax - kSpreadMin), kAccent,
           [this](double v) { act([v](Studio& s) { s.setSoundSpread(kSpreadMin + v * (kSpreadMax - kSpreadMin)); }); });
  }

  // [A][B][C][D]
  soundLitter_ = { x, kTilesY, w, kTileH };
  const int tw = (w - 3 * 8) / Sounds::kLitter;
  for (int i = 0; i < (int) view_.sounds.size(); ++i) paintSoundTile(g, i, { x + i * (tw + 8), kTilesY, tw, kTileH });

  paintSoundSettings(g, { x, kSettingsY, w, kSettingsH });
  paintPads(g, { x, kKeepY, w, kKeepH });
}

// Two cycles of the oscillator mix with the cutoff as a shaded floor (when `wave`), and a bar per setting.
void BloodlineSynth2Editor::paintPatch(juce::Graphics& g, const Patch& p, juce::Rectangle<int> area, float alpha, bool wave) {
  if (wave) {
    auto w = area.removeFromTop(area.getHeight() * 3 / 5).toFloat().reduced(0, 2);
    g.setColour(kBg.withAlpha(0.5f));
    g.fillRoundedRectangle(w, 4.0f);
    const double mixB = toReal(MixB, p[MixB]), ratio = std::exp2(toReal(TuneB, p[TuneB]) / 12.0);
    juce::Path path;
    for (int px = 0; px <= (int) w.getWidth(); ++px) {
      const double ph = 2.0 * px / w.getWidth();
      const double a = shapeAt(ph - std::floor(ph), toReal(ShapeA, p[ShapeA]));
      const double pb = ph * ratio;
      const double b = shapeAt(pb - std::floor(pb), toReal(ShapeB, p[ShapeB]));
      const float py = w.getCentreY() - (float) (((1.0 - mixB) * a + mixB * b) * w.getHeight() * 0.4);
      if (px == 0) path.startNewSubPath(w.getX(), py);
      else path.lineTo(w.getX() + px, py);
    }
    g.setColour(kAccent.withAlpha(alpha));
    g.strokePath(path, juce::PathStrokeType(1.5f));
    g.setColour(kGroupColour[1].withAlpha(0.18f));
    g.fillRect(w.withTrimmedTop(w.getHeight() * (1.0f - (1.0f - (float) p[Cutoff]) * 0.6f)));
  }
  auto bars = area.toFloat().reduced(0, 3);
  const float bw = bars.getWidth() / kNumSettings;
  for (int s = 0; s < kNumSettings; ++s) {
    const float h = juce::jmax(1.0f, (float) p[(size_t) s] * bars.getHeight());
    g.setColour(kGroupColour[groupOf(s)].withAlpha(alpha * 0.85f));
    g.fillRect(bars.getX() + s * bw, bars.getBottom() - h, juce::jmax(1.0f, bw - 1.0f), h);
  }
}

void BloodlineSynth2Editor::paintSoundTile(juce::Graphics& g, int i, juce::Rectangle<int> area) {
  const Patch& p = view_.sounds[(size_t) i];
  const bool heard = i == view_.soundSelected, parent = i == view_.soundParent;
  const float flash = heard ? juce::jmax(0.0f, 1.0f - view_.tileFlash / 8.0f) : 0.0f;
  const auto rect = area.toFloat();
  g.setColour((heard ? kTile.interpolatedWith(kAccent, 0.2f) : kTile).interpolatedWith(kFg, flash * 0.15f));
  g.fillRoundedRectangle(rect, 8.0f);
  g.setColour(heard ? kAccent : kEdge);
  g.drawRoundedRectangle(rect.reduced(heard ? 1.0f : 0.5f), 8.0f, heard ? 2.0f : 1.0f);
  Hit body{ area.withTrimmedBottom(32), [this, i] { proc_.hearSound(i); timerCallback(); } };
  body.carry = Carryable::SoundTile;
  body.index = i;
  hits_.push_back(body);

  auto in = area.reduced(8, 6);
  auto top = in.removeFromTop(20);
  g.setColour(kFg);
  g.setFont(font(17.0f, true));
  g.drawText(letter(i), top, juce::Justification::centredLeft);
  g.setFont(font(11.0f, true));
  if (parent) { g.setColour(kParent); g.drawText(star() + " parent", top, juce::Justification::centredRight); }
  else if (heard) { g.setColour(kAccent); g.drawText("hearing", top, juce::Justification::centredRight); }
  paintPatch(g, p, in.removeFromTop(80), heard ? 1.0f : 0.7f, true);
  g.setColour(kMuted);
  g.setFont(mono(10.0f));
  g.drawText(formatValue(Cutoff, p[Cutoff]) + "  " + formatValue(Attack, p[Attack]), in.removeFromTop(14), juce::Justification::centredLeft);

  auto actions = area.reduced(6, 5).removeFromBottom(26);
  const int half = actions.getWidth() / 2;
  button(g, actions.removeFromLeft(half), parent ? "Again" : "Breed", false, true, [this, i] { act([i](Studio& s) { s.sounds.breedFrom(i); }); });
  button(g, actions, "Cross", false, !heard, [this, i] { act([i](Studio& s) { s.sounds.cross(s.sounds.litter().selected(), i); }); });
}

// All 24 settings of the heard sound in two columns: Osc and Filter; Amp, Motion and Out.
void BloodlineSynth2Editor::paintSoundSettings(juce::Graphics& g, juce::Rectangle<int> area) {
  g.setColour(kPanel);
  g.fillRoundedRectangle(area.toFloat(), 8.0f);
  auto in = area.reduced(12, 8);
  const Patch& v = view_.sounds[(size_t) view_.soundSelected];
  auto head = in.removeFromTop(20);
  g.setColour(kFg);
  g.setFont(font(14.0f, true));
  g.drawText("HEARING " + letter(view_.soundSelected) + (view_.soundSelected == view_.soundParent ? "  " + star() : juce::String()),
             head.removeFromLeft(140), juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawText("Drag to change it. Locks keep a setting when breeding.", head, juce::Justification::centredRight);
  in.removeFromTop(4);

  const int colW = (in.getWidth() - 14) / 2;
  const int columns[2][3] = { { 0, 1, -1 }, { 2, 3, 4 } };
  for (int c = 0; c < 2; ++c) {
    auto col = juce::Rectangle<int>(in.getX() + c * (colW + 14), in.getY(), colW, in.getHeight());
    for (int gi : columns[c]) {
      if (gi < 0) continue;
      const Group& grp = group(gi);
      g.setColour(kGroupColour[gi]);
      g.setFont(font(11.0f, true));
      g.drawText(juce::String(grp.name).toUpperCase(), col.removeFromTop(18), juce::Justification::centredLeft);
      for (int s = grp.first; s < grp.first + grp.count; ++s) {
        auto r = col.removeFromTop(18);
        g.setColour(kMuted);
        g.setFont(font(11.0f));
        g.drawText(info(s).name, r.removeFromLeft(78), juce::Justification::centredLeft);
        g.setColour(kFg);
        g.setFont(mono(10.0f));
        g.drawText(formatValue(s, v[(size_t) s]), r.removeFromLeft(66), juce::Justification::centredLeft);
        auto lock = r.removeFromRight(50);
        r.removeFromRight(6);
        const auto track = r.reduced(0, 2);
        const auto bar = track.toFloat().withSizeKeepingCentre((float) track.getWidth(), 5.0f);
        g.setColour(kTile);
        g.fillRoundedRectangle(bar, 2.5f);
        if (view_.zone != Zone::None) {
          const synth::Range z = zoneRange(view_.zone, s);
          if (z.hi - z.lo < 1.0) {
            g.setColour(kGroupColour[gi].withAlpha(0.3f));
            g.fillRect(bar.getX() + bar.getWidth() * (float) z.lo, bar.getBottom() + 1.5f, bar.getWidth() * (float) (z.hi - z.lo), 2.0f);
          }
        }
        g.setColour(kGroupColour[gi]);
        g.fillRoundedRectangle(bar.withWidth(juce::jmax(2.0f, bar.getWidth() * (float) v[(size_t) s])), 2.5f);
        hits_.push_back({ track, {}, [this, s](double x) { act([=](Studio& st) { st.sounds.setValue(s, x); }); } });
        if (info(s).bred) {
          const bool locked = view_.locks[(size_t) s];
          button(g, lock, locked ? "Locked" : "Lock", locked, true, [this, s, locked] { act([=](Studio& st) { st.sounds.setLock(s, !locked); }); });
        }
      }
      col.removeFromTop(2);
    }
  }
}

void BloodlineSynth2Editor::paintPads(juce::Graphics& g, juce::Rectangle<int> area) {
  auto head = area.removeFromTop(16);
  label(g, head.removeFromLeft(60), "KEEP");
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawText("drag a sound here; the loops on the right play what you keep", head, juce::Justification::centredRight);
  area.removeFromTop(4);
  padAreas_.assign(Sounds::kPads, {});
  const int w = (area.getWidth() - 7 * 6) / Sounds::kPads;
  for (int p = 0; p < Sounds::kPads; ++p) {
    const auto r = juce::Rectangle<int>(area.getX() + p * (w + 6), area.getY(), w, area.getHeight());
    padAreas_[(size_t) p] = r;
    const Pad& pad = view_.pads[(size_t) p];
    const bool target = carry_ && carry_->moving && r.contains(carry_->at) &&
                        (carry_->what == Carryable::SoundTile || (carry_->what == Carryable::SoundPad && carry_->index != p));
    const bool used = view_.playing.sound == p && pad.full;
    const float flash = juce::jmax(0.0f, 1.0f - view_.padFlash[(size_t) p] / 8.0f);
    g.setColour((pad.full ? kTile : kPanel).interpolatedWith(kKeep, flash * 0.3f + (target ? 0.25f : 0.0f)));
    g.fillRoundedRectangle(r.toFloat(), 6.0f);
    g.setColour(target ? kKeep : pad.full ? zoneColour(pad.zone).withAlpha(used ? 1.0f : 0.6f) : kEdge);
    g.drawRoundedRectangle(r.toFloat().reduced(used ? 1.0f : 0.5f), 6.0f, target || used ? 2.0f : 1.0f);
    auto in = r.reduced(5, 4);
    auto top = in.removeFromTop(14);
    g.setColour(kMuted);
    g.setFont(mono(10.0f));
    g.drawText(juce::String(p + 1), top, juce::Justification::centredLeft);
    if (!pad.full) {
      g.drawText("drop", in, juce::Justification::centred);
      continue;
    }
    g.setColour(zoneColour(pad.zone));
    g.setFont(font(11.0f, true));
    g.drawFittedText(pad.name, in.removeFromTop(16), juce::Justification::centredLeft, 1, 0.7f);
    if (used) {
      g.setColour(kMuted);
      g.setFont(font(10.0f));
      g.drawText("in the loop", in.removeFromBottom(12), juce::Justification::centredLeft);
    }
    paintPatch(g, pad.sound, in.withTrimmedBottom(2), 0.8f, true);
    Hit h{ r, [this, p] { proc_.playPad(p); } };
    h.carry = Carryable::SoundPad;
    h.index = p;
    hits_.push_back(h);
    const auto x = juce::Rectangle<int>(r.getRight() - 17, r.getY() + 2, 15, 15);
    g.setColour(kMuted);
    g.setFont(font(12.0f, true));
    g.drawText(juce::String::fromUTF8("\xc3\x97"), x, juce::Justification::centred);
    hits_.push_back({ x, [this, p] { act([p](Studio& s) { s.clearPad(p); }); } });
  }
}

// ---------------------------------------------------------------------------------------------
// Right: the loops.

void BloodlineSynth2Editor::paintLoops(juce::Graphics& g, juce::Rectangle<int> area) {
  const int x = area.getX(), w = area.getWidth();
  // [bars][key][scale][back][new]
  label(g, { x, kRow1, 40, 26 }, "BARS");
  for (int b = 0; b < kNumBarChoices; ++b) {
    const int bars = kBarChoices[b];
    button(g, { x + 40 + b * 36, kRow1, 36, 26 }, juce::String(bars), view_.ops.bars == bars, true,
           [this, bars] { act([bars](Studio& s) { s.loops.setBars(bars); }); });
  }
  label(g, { x + 158, kRow1, 30, 26 }, "KEY");
  button(g, { x + 188, kRow1, 44, 26 }, keyName(view_.ops.key), false, true,
         [this] { act([k = view_.ops.key](Studio& s) { s.loops.setKey(k + 1); }); });
  label(g, { x + 242, kRow1, 44, 26 }, "SCALE");
  button(g, { x + 286, kRow1, 120, 26 }, scaleName(view_.ops.scale), false, true,
         [this] { act([sc = (int) view_.ops.scale](Studio& s) { s.loops.setScale((Scale) ((sc + 1) % kNumScales)); }); });
  button(g, { x + w - 140, kRow1, 64, 26 }, "Back", false, view_.loopBack, [this] { act([](Studio& s) { s.loops.back(); }); });
  button(g, { x + w - 72, kRow1, 72, 26 }, "New", false, true, [this] { act([](Studio& s) { s.loops.fresh(); }); });

  // [playing][midi][octaves][spread]
  {
    button(g, { x, kRow2, 70, 26 }, view_.play ? "Playing" : "Play", view_.play, true, [this] { proc_.setPlay(!view_.play); timerCallback(); });
    auto drag = juce::Rectangle<int>(x + 74, kRow2, 92, 26);
    g.setColour(kTile);
    g.fillRoundedRectangle(drag.toFloat().reduced(1.5f), 4.0f);
    g.setColour(kAccent);
    g.drawRoundedRectangle(drag.toFloat().reduced(2.0f), 4.0f, 1.5f);
    g.setColour(kFg);
    g.setFont(font(12.0f, true));
    g.drawText(juce::String::fromUTF8("\xe2\x87\xb2 Drag MIDI"), drag, juce::Justification::centred);
    Hit h{ drag };
    h.midiOut = true;
    hits_.push_back(h);

    // A dual slider, C0 to C8: the lowest and highest octave notes may use.
    label(g, { x + 176, kRow2, 64, 26 }, "OCTAVES");
    const auto track = juce::Rectangle<int>(x + 240, kRow2, 124, 26);
    const auto bar = track.toFloat().reduced(5.0f, 0).withSizeKeepingCentre(track.getWidth() - 10.0f, 6.0f);
    g.setColour(kTile);
    g.fillRoundedRectangle(bar, 3.0f);
    for (int o = 0; o <= 8; ++o) {
      g.setColour(kEdge);
      g.fillRect(bar.getX() + bar.getWidth() * o / 8.0f - 0.5f, bar.getBottom() + 2.0f, 1.0f, 3.0f);
    }
    const float lo = bar.getX() + bar.getWidth() * view_.ops.octLow / 8.0f, hi = bar.getX() + bar.getWidth() * view_.ops.octHigh / 8.0f;
    g.setColour(kParent);
    g.fillRect(lo, bar.getY(), hi - lo, bar.getHeight());
    g.fillEllipse(lo - 5.0f, bar.getCentreY() - 5.0f, 10.0f, 10.0f);
    g.fillEllipse(hi - 5.0f, bar.getCentreY() - 5.0f, 10.0f, 10.0f);
    hits_.push_back({ track, {}, [this](double v) {
      const int o = juce::jlimit(0, 8, juce::roundToInt(v * 8.0));
      if (octHandle_ < 0) octHandle_ = std::abs(o - view_.ops.octLow) <= std::abs(o - view_.ops.octHigh) ? 0 : 1;
      const int low = octHandle_ == 0 ? juce::jmin(o, view_.ops.octHigh - 1) : view_.ops.octLow;
      const int high = octHandle_ == 1 ? juce::jmax(o, view_.ops.octLow + 1) : view_.ops.octHigh;
      act([=](Studio& s) { s.loops.setOctaves(low, high); });
    } });
    g.setColour(kFg);
    g.setFont(mono(11.0f));
    g.drawText("C" + juce::String(view_.ops.octLow) + " to C" + juce::String(view_.ops.octHigh), x + 368, kRow2, 70, 26,
               juce::Justification::centredLeft);

    label(g, { x + 440, kRow2, 52, 26 }, "SPREAD");
    slider(g, { x + 494, kRow2, 80, 26 }, (view_.loopSpread - kSpreadMin) / (kSpreadMax - kSpreadMin), kAccent,
           [this](double v) { act([v](Studio& s) { s.setLoopSpread(kSpreadMin + v * (kSpreadMax - kSpreadMin)); }); });
    g.setColour(kFg);
    g.drawText(juce::String(view_.loopSpread, 2), x + w - 36, kRow2, 36, 26, juce::Justification::centredRight);
  }

  // [A][B][C][D]
  loopLitter_ = { x, kTilesY, w, kTileH };
  const int tw = (w - 3 * 8) / Loops::kLitter;
  for (int i = 0; i < (int) view_.loops.size(); ++i) paintLoopTile(g, i, { x + i * (tw + 8), kTilesY, tw, kTileH });

  paintLoopSettings(g, { x, kSettingsY, w, kSettingsH });
  paintSlots(g, { x, kKeepY, w, kKeepH });
}

// Pitch up, time across: the octave range top to bottom, bar lines, time locks as shaded bands.
void BloodlineSynth2Editor::paintRoll(juce::Graphics& g, const Loop& l, juce::Rectangle<int> area, bool live, bool big) {
  const auto a = area.toFloat();
  g.setColour(kBg.withAlpha(0.55f));
  g.fillRoundedRectangle(a, 4.0f);
  const int lo = view_.ops.lowNote(), hi = view_.ops.highNote();
  const float rows = (float) (hi - lo + 1), rh = a.getHeight() / rows;
  const double beats = l.beats();
  const float pxBeat = a.getWidth() / (float) beats;
  auto yOf = [&](int pitch) { return a.getBottom() - (juce::jlimit(lo, hi, pitch) - lo + 1) * rh; };

  if (big) {
    const bool tonal = view_.playing.traits[Tonal] < 0.5 && view_.ops.scale != Scale::Chromatic;
    for (int p = lo; p <= hi; ++p) {
      const bool root = (p - view_.ops.key) % 12 == 0;
      if (tonal && inScale(p, view_.ops.key, view_.ops.scale)) {
        g.setColour(kEdge.withAlpha(root ? 0.45f : 0.22f));
        g.fillRect(a.getX(), yOf(p), a.getWidth(), rh);
      }
      if (p % 12 == 0) {
        g.setColour(kEdge.withAlpha(0.7f));
        g.fillRect(a.getX(), yOf(p) + rh - 0.5f, a.getWidth(), 1.0f);
      }
    }
  }
  for (int b = 0; b <= (int) beats; ++b) {
    const bool bar = b % 4 == 0;
    if (!bar && !big) continue;
    g.setColour(kEdge.withAlpha(bar ? 0.9f : 0.35f));
    g.fillRect(a.getX() + b * pxBeat - 0.5f, a.getY(), 1.0f, a.getHeight());
  }
  for (const auto& r : view_.ops.timeLocks) {
    if (r.start >= beats) continue;
    const float x0 = a.getX() + (float) r.start * pxBeat, x1 = a.getX() + (float) std::min(r.end, beats) * pxBeat;
    g.setColour(kLock.withAlpha(0.14f));
    g.fillRect(x0, a.getY(), x1 - x0, a.getHeight());
  }
  if (big) noteRects_.clear();
  for (int i = 0; i < (int) l.notes.size(); ++i) {
    const Note& n = l.notes[(size_t) i];
    const float nx = a.getX() + (float) n.start * pxBeat;
    const float nw = juce::jmax(2.0f, (float) std::min(n.length, beats - n.start) * pxBeat - 1.0f);
    const juce::Rectangle<float> rect(nx, yOf(n.pitch) + (big ? 0.5f : 0.0f), nw, juce::jmax(1.5f, rh - (big ? 1.0f : 0.0f)));
    g.setColour(kAccent.withAlpha(live ? 1.0f : 0.8f).withMultipliedBrightness(0.5f + 0.5f * n.velocity));
    if (big) {
      g.fillRoundedRectangle(rect, 2.0f);
      g.setColour(kFg.withAlpha(0.5f));
      g.fillRect(rect.getRight() - 2.0f, rect.getY(), 2.0f, rect.getHeight()); // the edge that sets its length
      noteRects_.push_back({ rect, i });
    } else g.fillRect(rect);
  }
  if (live && view_.running) {
    const float px = a.getX() + (float) (view_.beat / beats) * a.getWidth();
    g.setColour(kFg.withAlpha(0.7f));
    g.fillRect(px, a.getY(), 1.5f, a.getHeight());
  }
}

void BloodlineSynth2Editor::paintLoopTile(juce::Graphics& g, int i, juce::Rectangle<int> area) {
  const Loop& l = view_.loops[(size_t) i];
  const bool heard = i == view_.loopSelected, live = heard && view_.playingSlot < 0, parent = i == view_.loopParent;
  const auto rect = area.toFloat();
  g.setColour(heard ? kTile.interpolatedWith(kAccent, live ? 0.18f : 0.07f) : kTile);
  g.fillRoundedRectangle(rect, 8.0f);
  g.setColour(live ? kAccent : heard ? kAccent.withAlpha(0.45f) : kEdge);
  g.drawRoundedRectangle(rect.reduced(live ? 1.0f : 0.5f), 8.0f, live ? 2.0f : 1.0f);
  Hit body{ area.withTrimmedBottom(32), [this, i] { act([i](Studio& s) { s.loops.select(i); }); } };
  body.carry = Carryable::LoopTile;
  body.index = i;
  hits_.push_back(body);

  auto in = area.reduced(8, 6);
  auto top = in.removeFromTop(20);
  g.setColour(kFg);
  g.setFont(font(17.0f, true));
  g.drawText(letter(i), top, juce::Justification::centredLeft);
  g.setFont(font(11.0f, true));
  if (parent) { g.setColour(kParent); g.drawText(star() + " parent", top, juce::Justification::centredRight); }
  else if (live) { g.setColour(kAccent); g.drawText("playing", top, juce::Justification::centredRight); }
  in.removeFromTop(2);
  paintRoll(g, l, in.removeFromTop(74), live, false);
  in.removeFromTop(2);
  g.setColour(l.sound >= 0 && view_.pads[(size_t) l.sound].full ? zoneColour(view_.pads[(size_t) l.sound].zone) : kMuted);
  g.setFont(mono(10.0f));
  g.drawText(soundName(l.sound) + "  " + juce::String((int) l.notes.size()) + " notes", in.removeFromTop(16), juce::Justification::centredLeft);

  auto actions = area.reduced(6, 5).removeFromBottom(26);
  const int half = actions.getWidth() / 2;
  button(g, actions.removeFromLeft(half), parent ? "Again" : "Breed", false, true, [this, i] { act([i](Studio& s) { s.loops.breedFrom(i); }); });
  button(g, actions, "Cross", false, !heard, [this, i] { act([i](Studio& s) { s.loops.cross(s.loops.litter().selected(), i); }); });
}

void BloodlineSynth2Editor::paintLoopSettings(juce::Graphics& g, juce::Rectangle<int> area) {
  g.setColour(kPanel);
  g.fillRoundedRectangle(area.toFloat(), 8.0f);
  auto in = area.reduced(12, 8);
  const Loop& l = view_.playing;
  const bool fromSlot = view_.playingSlot >= 0;
  auto head = in.removeFromTop(22);
  g.setColour(kFg);
  g.setFont(font(14.0f, true));
  g.drawText(fromSlot ? "PLAYING KEPT " + juce::String(view_.playingSlot + 1)
                      : "PLAYING " + letter(view_.loopSelected) + (view_.loopSelected == view_.loopParent ? "  " + star() : juce::String()),
             head.removeFromLeft(150), juce::Justification::centredLeft);
  // The sound chip: click to cycle through kept sounds, or drop a kept pad on it.
  soundChip_ = head.removeFromLeft(150).reduced(0, 1);
  {
    const bool has = l.sound >= 0 && view_.pads[(size_t) l.sound].full;
    const bool target = carry_ && carry_->moving && carry_->what == Carryable::SoundPad && soundChip_.contains(carry_->at);
    const auto f = soundChip_.toFloat();
    g.setColour(target ? kKeep.withAlpha(0.3f) : kTile);
    g.fillRoundedRectangle(f, 10.0f);
    g.setColour(target ? kKeep : has ? zoneColour(view_.pads[(size_t) l.sound].zone) : kEdge);
    g.drawRoundedRectangle(f.reduced(0.5f), 10.0f, 1.5f);
    g.setFont(font(12.0f, true));
    g.drawFittedText((has ? juce::String::fromUTF8("\xe2\x99\xaa ") : juce::String()) + soundName(l.sound), soundChip_, juce::Justification::centred, 1);
    hits_.push_back({ soundChip_, [this] { act([](Studio& s) { s.loops.cycleSound(); }); } });
  }
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawFittedText("Click to add, drag to move, drag the end to lengthen, click to remove. Shift snaps.", head.withTrimmedLeft(8),
                   juce::Justification::centredRight, 2, 0.8f);
  in.removeFromTop(4);

  // The piano roll, pitch names on the left.
  auto rollRow = in.removeFromTop(150);
  auto names = rollRow.removeFromLeft(30);
  roll_ = rollRow;
  paintRoll(g, l, roll_, true, true);
  {
    const int lo = view_.ops.lowNote(), hi = view_.ops.highNote();
    const float rh = roll_.getHeight() / (float) (hi - lo + 1);
    g.setColour(kMuted);
    g.setFont(mono(9.0f));
    for (int p = lo; p <= hi; p += 12)
      g.drawText(noteName(p), names.getX(), (int) (roll_.getBottom() - (p - lo + 1) * rh - 4), 28, 12, juce::Justification::centredLeft);
  }

  // The lock strip, the loop's full length.
  in.removeFromTop(4);
  auto stripRow = in.removeFromTop(16);
  label(g, stripRow.removeFromLeft(30), "LOCK");
  strip_ = stripRow;
  {
    const auto a = strip_.toFloat();
    g.setColour(kTile);
    g.fillRoundedRectangle(a, 3.0f);
    const double beats = l.beats();
    const float pxBeat = a.getWidth() / (float) beats;
    for (int b = 0; b <= (int) beats; ++b) {
      g.setColour(kEdge.withAlpha(b % 4 == 0 ? 0.9f : 0.4f));
      g.fillRect(a.getX() + b * pxBeat - 0.5f, a.getY() + (b % 4 == 0 ? 0.0f : 5.0f), 1.0f, b % 4 == 0 ? a.getHeight() : a.getHeight() - 10.0f);
    }
    auto band = [&](double s, double e, float alpha) {
      if (s >= beats) return;
      const float x0 = a.getX() + (float) s * pxBeat, x1 = a.getX() + (float) std::min(e, beats) * pxBeat;
      g.setColour(kLock.withAlpha(alpha));
      g.fillRoundedRectangle(x0, a.getY() + 1.0f, juce::jmax(2.0f, x1 - x0), a.getHeight() - 2.0f, 3.0f);
    };
    for (const auto& r : view_.ops.timeLocks) band(r.start, r.end, 0.75f);
    if (stripDrag_ && stripDrag_->moved) band(std::min(stripDrag_->from, stripDrag_->to), std::max(stripDrag_->from, stripDrag_->to), 0.4f);
    if (view_.ops.timeLocks.empty() && !stripDrag_) {
      g.setColour(kMuted);
      g.setFont(font(10.0f));
      g.drawText("drag here to lock a stretch of time for breeding", strip_, juce::Justification::centred);
    }
  }

  // The traits, two columns of four.
  in.removeFromTop(8);
  const int colW = (in.getWidth() - 16) / 2;
  for (int t = 0; t < kNumTraits; ++t) {
    const int col = t / 4, row = t % 4;
    auto r = juce::Rectangle<int>(in.getX() + col * (colW + 16), in.getY() + row * 24, colW, 22);
    const TraitInfo& ti = traitInfo(t);
    const bool locked = view_.ops.traitLocks[(size_t) t];
    auto lock = r.removeFromRight(48);
    r.removeFromRight(6);
    g.setColour(kMuted);
    g.setFont(font(11.0f));
    g.drawText(ti.low, r.removeFromLeft(62), juce::Justification::centredLeft);
    g.drawText(ti.high, r.removeFromRight(62), juce::Justification::centredRight);
    slider(g, r.reduced(6, 0), l.traits[(size_t) t], kTraitColour[t], [this, t](double v) { act([=](Studio& s) { s.loops.setTrait(t, v); }); },
           [this] { act([](Studio& s) { s.loops.redraw(); }); });
    button(g, lock, locked ? "Locked" : "Lock", locked, true, [this, t, locked] { act([=](Studio& s) { s.loops.setTraitLock(t, !locked); }); });
  }
  in.removeFromTop(4 * 24 + 2);

  // A line of guidance.
  bool anyKept = false;
  for (const auto& p : view_.pads) anyKept |= p.full;
  juce::String note;
  juce::Colour colour = kMuted;
  if (!anyKept) { note = "Keep a sound on the left to hear the loops."; colour = kParent; }
  else if (view_.draggedOut && !view_.play) { note = "Playing off: the clip plays the synth. Press Play to hear the loops here again."; colour = kParent; }
  else if (view_.riff == BloodlineSynth2Processor::InLoop) { note = "In loop: the loop plays the sound you are hearing on the left."; colour = kAccent; }
  else
    note = "Key " + juce::String(keyName(view_.ops.key)) + " " + scaleName(view_.ops.scale) + " guides Tonal. Traits redraw on release. Live is at " +
           juce::String(juce::roundToInt(view_.bpm)) + " BPM.";
  g.setColour(colour);
  g.setFont(font(11.0f));
  g.drawFittedText(note, in, juce::Justification::centredLeft, 1, 0.8f);
}

void BloodlineSynth2Editor::paintSlots(juce::Graphics& g, juce::Rectangle<int> area) {
  auto head = area.removeFromTop(16);
  label(g, head.removeFromLeft(60), "KEEP");
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawText("drag a loop here; click to play it", head, juce::Justification::centredRight);
  area.removeFromTop(4);
  slotAreas_.assign(Loops::kSlots, {});
  const int w = (area.getWidth() - 7 * 6) / Loops::kSlots;
  for (int s = 0; s < Loops::kSlots; ++s) {
    const auto r = juce::Rectangle<int>(area.getX() + s * (w + 6), area.getY(), w, area.getHeight());
    slotAreas_[(size_t) s] = r;
    const Slot& slot = view_.slots[(size_t) s];
    const bool live = view_.playingSlot == s;
    const bool target = carry_ && carry_->moving && r.contains(carry_->at) &&
                        (carry_->what == Carryable::LoopTile || (carry_->what == Carryable::LoopSlot && carry_->index != s));
    g.setColour((slot.full ? kTile : kPanel).interpolatedWith(kKeep, target ? 0.25f : 0.0f).interpolatedWith(kAccent, live ? 0.18f : 0.0f));
    g.fillRoundedRectangle(r.toFloat(), 6.0f);
    g.setColour(target ? kKeep : live ? kAccent : kEdge);
    g.drawRoundedRectangle(r.toFloat().reduced(live ? 1.0f : 0.5f), 6.0f, target || live ? 2.0f : 1.0f);
    auto in = r.reduced(5, 4);
    auto top = in.removeFromTop(14);
    g.setColour(live ? kAccent : kMuted);
    g.setFont(mono(10.0f));
    g.drawText(juce::String(s + 1) + (slot.full ? "  " + juce::String(slot.loop.bars) + " bars" : juce::String()), top, juce::Justification::centredLeft);
    if (!slot.full) {
      g.drawText("drop", in, juce::Justification::centred);
      continue;
    }
    g.setFont(font(10.0f));
    g.drawFittedText(soundName(slot.loop.sound), in.removeFromBottom(14), juce::Justification::centredLeft, 1, 0.7f);
    in.removeFromTop(2);
    paintRoll(g, slot.loop, in.withTrimmedBottom(2), live, false);
    Hit h{ r, [this, s] { act([s](Studio& st) { st.loops.playSlot(s); }); } };
    h.carry = Carryable::LoopSlot;
    h.index = s;
    hits_.push_back(h);
    const auto x = juce::Rectangle<int>(r.getRight() - 17, r.getY() + 2, 15, 15);
    g.setColour(kMuted);
    g.setFont(font(12.0f, true));
    g.drawText(juce::String::fromUTF8("\xc3\x97"), x, juce::Justification::centred);
    hits_.push_back({ x, [this, s] { act([s](Studio& st) { st.loops.clearSlot(s); }); } });
  }
}

// ---------------------------------------------------------------------------------------------
// Carrying tiles and kept things about.

void BloodlineSynth2Editor::paintCarry(juce::Graphics& g) {
  if (!carry_ || !carry_->moving) return;
  const auto what = carry_->what;
  const int i = carry_->index;
  juce::String text;
  if (what == Carryable::SoundTile || what == Carryable::LoopTile) text = letter(i);
  else if (what == Carryable::SoundPad) text = view_.pads[(size_t) i].name;
  else text = "Kept " + juce::String(i + 1);
  const juce::Rectangle<int> home = what == Carryable::SoundPad ? soundLitter_ : what == Carryable::LoopSlot ? loopLitter_ : juce::Rectangle<int>();
  if (!home.isEmpty() && home.contains(carry_->at)) {
    g.setColour(kParent.withAlpha(0.12f));
    g.fillRoundedRectangle(home.toFloat(), 8.0f);
    g.setColour(kParent);
    g.setFont(font(14.0f, true));
    g.drawText("drop to breed from " + text, home.withHeight(28), juce::Justification::centred);
  }
  auto ghost = juce::Rectangle<float>(0, 0, 90, 36).withCentre(carry_->at.toFloat());
  g.setColour(kKeep.withAlpha(0.85f));
  g.fillRoundedRectangle(ghost, 6.0f);
  g.setColour(kBg);
  g.setFont(font(13.0f, true));
  g.drawFittedText(text, ghost.toNearestInt(), juce::Justification::centred, 1);
}

int BloodlineSynth2Editor::indexAt(const std::vector<juce::Rectangle<int>>& areas, juce::Point<int> p) const {
  for (int i = 0; i < (int) areas.size(); ++i)
    if (areas[(size_t) i].contains(p)) return i;
  return -1;
}

double BloodlineSynth2Editor::beatAt(int x) const {
  return (x - roll_.getX()) / (double) juce::jmax(1, roll_.getWidth()) * view_.playing.beats();
}

int BloodlineSynth2Editor::pitchAt(int y) const {
  const int lo = view_.ops.lowNote(), hi = view_.ops.highNote();
  const double rh = roll_.getHeight() / (double) (hi - lo + 1);
  return juce::jlimit(lo, hi, lo + (int) std::floor((roll_.getBottom() - y) / rh));
}

bool BloodlineSynth2Editor::rollMouseDown(const juce::MouseEvent& e) {
  const auto pos = e.getPosition();
  if (!roll_.contains(pos)) return false;
  for (auto it = noteRects_.rbegin(); it != noteRects_.rend(); ++it) {
    if (!it->first.expanded(1.0f, 0.0f).contains(pos.toFloat())) continue;
    const int i = it->second;
    if (i >= (int) view_.playing.notes.size()) break;
    const bool resize = pos.x >= it->first.getRight() - 5.0f;
    noteDrag_ = NoteDrag{ i, resize, false, false, view_.playing.notes[(size_t) i], pos };
    return true;
  }
  const double beat = juce::jlimit(0.0, view_.playing.beats() - kSixteenth, std::round(beatAt(pos.x) / kSixteenth) * kSixteenth);
  const int pitch = pitchAt(pos.y);
  int added = -1;
  act([&](Studio& s) { added = s.loops.addNote(beat, pitch); });
  if (added >= 0) noteDrag_ = NoteDrag{ added, false, true, false, view_.playing.notes[(size_t) added], pos };
  return true;
}

bool BloodlineSynth2Editor::stripMouseDown(const juce::MouseEvent& e) {
  const auto pos = e.getPosition();
  if (!strip_.contains(pos)) return false;
  const double beat = (pos.x - strip_.getX()) / (double) juce::jmax(1, strip_.getWidth()) * view_.playing.beats();
  bool onLock = false;
  for (const auto& r : view_.ops.timeLocks) onLock |= beat >= r.start && beat < r.end;
  stripDrag_ = StripDrag{ beat, beat, onLock, false };
  return true;
}

void BloodlineSynth2Editor::mouseDown(const juce::MouseEvent& e) {
  const auto pos = e.getPosition();
  carry_.reset();
  noteDrag_.reset();
  stripDrag_.reset();
  octHandle_ = -1;
  midiDragStarted_ = false;
  for (auto it = hits_.rbegin(); it != hits_.rend(); ++it) {
    if (!it->area.contains(pos)) continue;
    Hit h = *it;
    if (h.midiOut) { dragging_ = h; return; }
    if (h.drag) { dragging_ = h; h.drag(juce::jlimit(0.0, 1.0, (pos.x - h.area.getX()) / (double) juce::jmax(1, h.area.getWidth()))); return; }
    if (h.carry != Carryable::None) carry_ = Carry{ h.carry, h.index, pos };
    if (h.click) h.click();
    return;
  }
  if (rollMouseDown(e) || stripMouseDown(e)) repaint();
}

void BloodlineSynth2Editor::mouseDrag(const juce::MouseEvent& e) {
  if (carry_) {
    carry_->at = e.getPosition();
    if (e.getDistanceFromDragStart() > 6) carry_->moving = true;
    repaint();
    return;
  }
  if (noteDrag_) {
    if (e.getDistanceFromDragStart() > 3) noteDrag_->moved = true;
    if (!noteDrag_->moved) return;
    const auto d = e.getPosition() - noteDrag_->down;
    const double beats = view_.playing.beats();
    const double dBeat = d.x / (double) juce::jmax(1, roll_.getWidth()) * beats;
    const int lo = view_.ops.lowNote(), hi = view_.ops.highNote();
    const double rh = roll_.getHeight() / (double) (hi - lo + 1);
    Note n = noteDrag_->orig;
    if (noteDrag_->resize) {
      double end = n.start + n.length + dBeat;
      if (e.mods.isShiftDown()) end = snapSixteenth(end);
      n.length = juce::jlimit(kMinLength, beats - n.start, end - n.start);
    } else {
      double start = n.start + dBeat;
      if (e.mods.isShiftDown()) start = snapSixteenth(start);
      n.start = juce::jlimit(0.0, beats - 1e-6, start);
      n.pitch = juce::jlimit(lo, hi, n.pitch - (int) std::round(d.y / rh));
    }
    const int i = noteDrag_->index;
    act([=](Studio& s) { s.loops.setNote(i, n); });
    return;
  }
  if (stripDrag_) {
    if (e.getDistanceFromDragStart() > 3) stripDrag_->moved = true;
    double beat = (e.getPosition().x - strip_.getX()) / (double) juce::jmax(1, strip_.getWidth()) * view_.playing.beats();
    beat = juce::jlimit(0.0, view_.playing.beats(), beat);
    stripDrag_->to = beat;
    repaint();
    return;
  }
  if (!dragging_) return;
  if (dragging_->midiOut) {
    if (midiDragStarted_ || e.getDistanceFromDragStart() < 6) return;
    midiDragStarted_ = true;
    const juce::File f = proc_.writeMidiFile();
    if (f.existsAsFile()) juce::DragAndDropContainer::performExternalDragDropOfFiles({ f.getFullPathName() }, false, this);
    return;
  }
  const auto& a = dragging_->area;
  dragging_->drag(juce::jlimit(0.0, 1.0, (e.getPosition().x - a.getX()) / (double) juce::jmax(1, a.getWidth())));
}

void BloodlineSynth2Editor::mouseUp(const juce::MouseEvent& e) {
  if (dragging_ && dragging_->release) dragging_->release();
  dragging_.reset();
  octHandle_ = -1;
  if (noteDrag_) {
    const NoteDrag nd = *noteDrag_;
    noteDrag_.reset();
    if (!nd.moved && !nd.added) act([i = nd.index](Studio& s) { s.loops.removeNote(i); });
    repaint();
    return;
  }
  if (stripDrag_) {
    const StripDrag sd = *stripDrag_;
    stripDrag_.reset();
    if (!sd.moved) {
      if (sd.onLock) act([b = sd.from](Studio& s) { s.loops.removeTimeLockAt(b); });
    } else {
      double a = std::min(sd.from, sd.to), b = std::max(sd.from, sd.to);
      if (!e.mods.isShiftDown()) {
        a = std::floor(a / kSixteenth + 0.5) * kSixteenth;
        b = std::max(a + kSixteenth, std::floor(b / kSixteenth + 0.5) * kSixteenth);
      }
      act([=](Studio& s) { s.loops.addTimeLock(a, b); });
    }
    repaint();
    return;
  }
  if (!carry_ || !carry_->moving) { carry_.reset(); return; }
  const Carry c = *carry_;
  carry_.reset();
  const auto at = e.getPosition();
  const int pad = indexAt(padAreas_, at), slot = indexAt(slotAreas_, at);
  const int i = c.index;
  switch (c.what) {
    case Carryable::SoundTile:
      if (pad >= 0) act([=](Studio& s) { s.keepSound(i, pad); });
      break;
    case Carryable::SoundPad:
      if (pad >= 0 && pad != i) act([=](Studio& s) { s.swapPads(i, pad); });
      else if (soundLitter_.contains(at)) act([=](Studio& s) { s.sounds.breedFromPad(i); });
      else if (soundChip_.contains(at)) act([=](Studio& s) { s.loops.setSound(i); });
      break;
    case Carryable::LoopTile:
      if (slot >= 0) act([=](Studio& s) { s.loops.keep(i, slot); });
      break;
    case Carryable::LoopSlot:
      if (slot >= 0 && slot != i) act([=](Studio& s) { s.loops.swapSlots(i, slot); });
      else if (loopLitter_.contains(at)) act([=](Studio& s) { s.loops.breedFromSlot(i); });
      break;
    case Carryable::None: break;
  }
  repaint();
}
