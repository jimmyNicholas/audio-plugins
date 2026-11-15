#include "editor.h"
#include <cmath>

using namespace bb2;
using namespace bloodline;

namespace {

const juce::Colour kBg(0xff15171c), kPanel(0xff1f2229), kTile(0xff292d36), kEdge(0xff3a3f4b);
const juce::Colour kFg(0xffe8e6e1), kMuted(0xff8d8f96);
const juce::Colour kAccent(0xffff8c69), kParent(0xffffc857), kKeep(0xff9be564), kLock(0xffb39ddb), kWild(0xffc9c6bf);

const juce::Colour kGroupColour[kNumGroups] = {
  juce::Colour(0xff7cc6fe), juce::Colour(0xffff8c69), juce::Colour(0xff9be564), juce::Colour(0xffd59bf6), juce::Colour(0xfff4d35e),
};
// One colour a lane: kick, snare, hat, perc. The lanes carry no names; colour and place say it.
const juce::Colour kLaneColour[kLanes] = {
  juce::Colour(0xffff6b6b), juce::Colour(0xff7cc6fe), juce::Colour(0xfff4d35e), juce::Colour(0xff9be564),
};
juce::Colour zoneColour(bb2::Zone z) { return z == bb2::Zone::None ? kWild : kLaneColour[laneFor(z)]; }

const bb2::Zone kZones[bb2::kNumZones] = { bb2::Zone::None, bb2::Zone::Kick, bb2::Zone::Snare, bb2::Zone::Hats, bb2::Zone::Perc };
const char* kLayerName[3] = { "One", "Stack", "Flam" };

juce::Font font(float size, bool bold = false) { return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain)); }
juce::Font mono(float size) { return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain)); }
juce::String letter(int i) { return juce::String::charToString((juce::juce_wchar) ('A' + i)); }
juce::String num(double v, int places) { return places == 0 ? juce::String(juce::roundToInt(v)) : juce::String(v, places); }
juce::String pct(double v) { return juce::String(juce::roundToInt(v * 100.0)) + "%"; }
juce::String star() { return juce::String::fromUTF8("\xe2\x98\x85"); }

juce::String pullName(double p) {
  if (p < 0.2) return "loose";
  if (p < 0.45) return "soft";
  if (p < 0.75) return "firm";
  return "tight";
}

int groupOf(int s) {
  for (int g = 0; g < kNumGroups; ++g)
    if (s >= group(g).first && s < group(g).first + group(g).count) return g;
  return 0;
}

juce::String formatValue(int s, double norm) {
  const double v = toReal(s, norm);
  const juce::String unit(info(s).unit);
  if (s == Pan) return std::abs(v) < 0.02 ? juce::String("C") : (v < 0 ? "L" : "R") + num(std::abs(v) * 100.0, 0);
  if (s == Bits) return juce::String(v, 1) + " bits";
  if (unit == "Hz") return v >= 1000 ? juce::String(v / 1000.0, 1) + " kHz" : num(v, 0) + " Hz";
  if (unit == "s") return v < 1.0 ? num(v * 1000.0, v < 0.01 ? 1 : 0) + " ms" : juce::String(v, 2) + " s";
  if (unit == "st") return juce::String(v, 1) + " st";
  if (unit == "dB") return juce::String(v, 1) + " dB";
  return juce::String(juce::roundToInt(v * 100.0)) + "%";
}

// Spread runs 0.02 to 0.5 in both litters.
constexpr double kSpreadMin = 0.02, kSpreadMax = 0.5;

constexpr int kWidth = 1280, kHeight = 700, kPanelW = 616, kRow1 = 46, kRow2 = 80, kTilesY = 120, kTileH = 152;
constexpr int kSettingsY = 282, kSettingsH = 276, kKeepY = 570, kKeepH = 118;
constexpr int kHead = 226, kLaneH = 36; // the right panel's settings: lane heads, lane height

} // namespace

BloodlineBreaks2Editor::BloodlineBreaks2Editor(BloodlineBreaks2Processor& p) : AudioProcessorEditor(&p), proc_(p), view_(p.view()) {
  setSize(kWidth, kHeight);
  startTimerHz(30);
}

BloodlineBreaks2Editor::~BloodlineBreaks2Editor() { stopTimer(); }

void BloodlineBreaks2Editor::timerCallback() {
  view_ = proc_.view();
  repaint();
}

void BloodlineBreaks2Editor::act(const std::function<void(Studio&)>& change) {
  proc_.edit(change);
  timerCallback();
}

void BloodlineBreaks2Editor::button(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled,
                                    std::function<void()> click, juce::Colour onColour) {
  const auto f = r.toFloat().reduced(1.5f);
  const float a = enabled ? 1.0f : 0.35f;
  g.setColour((on ? onColour : kTile).withMultipliedAlpha(a));
  g.fillRoundedRectangle(f, 4.0f);
  g.setColour((on ? onColour : kEdge).withMultipliedAlpha(a));
  g.drawRoundedRectangle(f.reduced(0.5f), 4.0f, 1.0f);
  g.setColour((on ? kBg : kFg).withMultipliedAlpha(a));
  g.setFont(font(12.0f, on));
  g.drawFittedText(text, r.reduced(2, 0), juce::Justification::centred, 1, 0.7f);
  if (enabled) hits_.push_back({ r, std::move(click) });
}

void BloodlineBreaks2Editor::slider(juce::Graphics& g, juce::Rectangle<int> r, double value, juce::Colour c, std::function<void(double)> drag) {
  const auto bar = r.toFloat().withSizeKeepingCentre((float) r.getWidth(), 6.0f);
  g.setColour(kTile);
  g.fillRoundedRectangle(bar, 3.0f);
  g.setColour(c);
  g.fillRoundedRectangle(bar.withWidth(juce::jmax(3.0f, bar.getWidth() * (float) value)), 3.0f);
  g.fillEllipse(bar.getX() + bar.getWidth() * (float) value - 5.0f, bar.getCentreY() - 5.0f, 10.0f, 10.0f);
  hits_.push_back({ r, {}, std::move(drag) });
}

void BloodlineBreaks2Editor::label(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text) {
  g.setColour(kMuted);
  g.setFont(font(11.0f, true));
  g.drawText(text, r, juce::Justification::centredLeft);
}

juce::String BloodlineBreaks2Editor::padName(int pad) const {
  if (pad < 0 || pad >= Drums::kPads || !view_.pads[(size_t) pad].full) return "none";
  return view_.pads[(size_t) pad].name;
}

// A slider for a litter's spread, with its value beside it.
void BloodlineBreaks2Editor::spreadSlider(juce::Graphics& g, juce::Rectangle<int> r, double spread, std::function<void(double)> set) {
  label(g, r.removeFromLeft(48), "SPREAD");
  auto value = r.removeFromRight(36);
  slider(g, r.withTrimmedRight(6), (spread - kSpreadMin) / (kSpreadMax - kSpreadMin), kAccent,
         [set](double v) { set(kSpreadMin + v * (kSpreadMax - kSpreadMin)); });
  g.setColour(kFg);
  g.setFont(mono(11.0f));
  g.drawText(juce::String(spread, 2), value, juce::Justification::centredRight);
}

void BloodlineBreaks2Editor::paint(juce::Graphics& g) {
  hits_.clear();
  g.fillAll(kBg);
  g.setColour(kFg);
  g.setFont(font(18.0f, true));
  g.drawText("BLOODLINE BREAKS 2", 16, 10, 240, 28, juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(mono(11.0f));
  g.drawText("prototype  |  breed drums on the left, keep them, then breed loops that play them on the right", 250, 10, 800, 28,
             juce::Justification::centredLeft);
  g.setColour(kEdge);
  g.fillRect(16 + kPanelW + 7, kRow1, 2, kHeight - kRow1 - 12);
  paintDrums(g, { 16, 0, kPanelW, kHeight });
  paintLoops(g, { 16 + kPanelW + 16, 0, kPanelW, kHeight });
  paintCarry(g);
}

// ---------------------------------------------------------------------------------------------
// Left: the drums.

void BloodlineBreaks2Editor::paintDrums(juce::Graphics& g, juce::Rectangle<int> area) {
  const int x = area.getX(), w = area.getWidth();
  // [zone][back][new]
  label(g, { x, kRow1, 44, 26 }, "ZONE");
  for (int z = 0; z < bb2::kNumZones; ++z)
    button(g, { x + 44 + z * 64, kRow1, 64, 26 }, bb2::zoneName(kZones[z]), view_.zone == kZones[z], true,
           [this, z] { act([z](Studio& s) { s.drums.setZone(kZones[z]); }); });
  button(g, { x + w - 140, kRow1, 64, 26 }, "Back", false, view_.drumBack, [this] { act([](Studio& s) { s.drums.back(); }); });
  button(g, { x + w - 72, kRow1, 72, 26 }, "New", false, true, [this] { act([](Studio& s) { s.drums.fresh(); }); });

  // [loop][spread]
  label(g, { x, kRow2, 44, 26 }, "LOOP");
  button(g, { x + 44, kRow2, 46, 26 }, view_.auditionOn ? "On" : "Off", view_.auditionOn, true,
         [this] { proc_.setAudition(!view_.auditionOn, view_.audition); timerCallback(); });
  for (int p = 0; p < Audition::kPatterns; ++p)
    button(g, { x + 92 + p * 66, kRow2, 66, 26 }, Audition::patternName(p), view_.audition == p, true,
           [this, p] { proc_.setAudition(true, p); timerCallback(); });
  spreadSlider(g, { x + 364, kRow2, w - 364, 26 }, view_.drumSpread, [this](double s) { act([s](Studio& st) { st.drums.setSpread(s); }); });

  // [A][B][C][D]
  drumLitter_ = { x, kTilesY, w, kTileH };
  const int tw = (w - 3 * 8) / Drums::kLitter;
  for (int i = 0; i < (int) view_.drums.size(); ++i) paintDrumTile(g, i, { x + i * (tw + 8), kTilesY, tw, kTileH });

  paintDrumSettings(g, { x, kSettingsY, w, kSettingsH });
  paintPads(g, { x, kKeepY, w, kKeepH });
}

// The amplitude of body and noise over the first half second, and a bar per setting.
void BloodlineBreaks2Editor::paintSound(juce::Graphics& g, const Values& v, juce::Rectangle<int> area, float alpha) {
  auto env = area.removeFromTop(area.getHeight() / 2).toFloat().reduced(0, 2);
  g.setColour(kBg.withAlpha(0.5f));
  g.fillRoundedRectangle(env, 4.0f);
  const double decay = toReal(Decay, v[Decay]), noiseDecay = toReal(NoiseDecay, v[NoiseDecay]), mix = v[BodyNoise];
  juce::Path body, noise;
  for (int px = 0; px <= (int) env.getWidth(); ++px) {
    const double t = 0.5 * px / env.getWidth();
    const float yb = env.getBottom() - (float) ((1.0 - mix) * std::pow(0.001, t / decay)) * env.getHeight() * 0.9f;
    const float yn = env.getBottom() - (float) (mix * std::pow(0.001, t / noiseDecay)) * env.getHeight() * 0.9f;
    if (px == 0) { body.startNewSubPath(env.getX(), yb); noise.startNewSubPath(env.getX(), yn); }
    else { body.lineTo(env.getX() + px, yb); noise.lineTo(env.getX() + px, yn); }
  }
  g.setColour(kGroupColour[0].withAlpha(alpha));
  g.strokePath(body, juce::PathStrokeType(1.5f));
  g.setColour(kGroupColour[1].withAlpha(alpha));
  g.strokePath(noise, juce::PathStrokeType(1.5f));

  auto bars = area.toFloat().reduced(0, 3);
  const float bw = bars.getWidth() / kNumSettings;
  for (int s = 0; s < kNumSettings; ++s) {
    const float h = juce::jmax(1.0f, (float) v[(size_t) s] * bars.getHeight());
    g.setColour(kGroupColour[groupOf(s)].withAlpha(alpha * 0.85f));
    g.fillRect(bars.getX() + s * bw, bars.getBottom() - h, juce::jmax(1.0f, bw - 1.0f), h);
  }
}

void BloodlineBreaks2Editor::paintDrumTile(juce::Graphics& g, int i, juce::Rectangle<int> area) {
  const Values& v = view_.drums[(size_t) i];
  const bool heard = i == view_.drumSelected, parent = i == view_.drumParent;
  const float flash = heard ? juce::jmax(0.0f, 1.0f - view_.tileFlash / 8.0f) : 0.0f;
  const auto rect = area.toFloat();
  g.setColour((heard ? kTile.interpolatedWith(kAccent, 0.2f) : kTile).interpolatedWith(kFg, flash * 0.15f));
  g.fillRoundedRectangle(rect, 8.0f);
  g.setColour(heard ? kAccent : kEdge);
  g.drawRoundedRectangle(rect.reduced(heard ? 1.0f : 0.5f), 8.0f, heard ? 2.0f : 1.0f);
  Hit body{ area.withTrimmedBottom(32), [this, i] { proc_.hearDrum(i); timerCallback(); } };
  body.carry = Carryable::DrumTile;
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
  paintSound(g, v, in.removeFromTop(78), heard ? 1.0f : 0.7f);
  g.setColour(kMuted);
  g.setFont(mono(10.0f));
  g.drawText(formatValue(Pitch, v[Pitch]) + "  " + formatValue(Decay, v[Decay]), in.removeFromTop(16), juce::Justification::centredLeft);

  auto actions = area.reduced(6, 5).removeFromBottom(26);
  const int half = actions.getWidth() / 2;
  button(g, actions.removeFromLeft(half), parent ? "Again" : "Breed", false, true, [this, i] { act([i](Studio& s) { s.drums.breedFrom(i); }); });
  button(g, actions, "Cross", false, !heard, [this, i] { act([i](Studio& s) { s.drums.cross(s.drums.litter().selected(), i); }); });
}

// Every setting of the heard drum in two columns: Body and Noise, then Click, Grit and Out.
void BloodlineBreaks2Editor::paintDrumSettings(juce::Graphics& g, juce::Rectangle<int> area) {
  g.setColour(kPanel);
  g.fillRoundedRectangle(area.toFloat(), 8.0f);
  auto in = area.reduced(12, 8);
  const Values& v = view_.drums[(size_t) view_.drumSelected];
  auto head = in.removeFromTop(20);
  g.setColour(kFg);
  g.setFont(font(14.0f, true));
  g.drawText("HEARING " + letter(view_.drumSelected) + (view_.drumSelected == view_.drumParent ? "  " + star() : juce::String()),
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
        g.drawText(info(s).name, r.removeFromLeft(76), juce::Justification::centredLeft);
        g.setColour(kFg);
        g.setFont(mono(10.0f));
        g.drawText(formatValue(s, v[(size_t) s]), r.removeFromLeft(60), juce::Justification::centredLeft);
        auto lock = r.removeFromRight(50);
        r.removeFromRight(6);
        const auto track = r.reduced(0, 2);
        const auto bar = track.toFloat().withSizeKeepingCentre((float) track.getWidth(), 5.0f);
        g.setColour(kTile);
        g.fillRoundedRectangle(bar, 2.5f);
        if (view_.zone != bb2::Zone::None) {
          const Range z = bb2::zoneRange(view_.zone, s);
          if (z.hi - z.lo < 1.0) {
            g.setColour(kGroupColour[gi].withAlpha(0.3f));
            g.fillRect(bar.getX() + bar.getWidth() * (float) z.lo, bar.getBottom() + 1.5f, bar.getWidth() * (float) (z.hi - z.lo), 2.0f);
          }
        }
        g.setColour(kGroupColour[gi]);
        g.fillRoundedRectangle(bar.withWidth(juce::jmax(2.0f, bar.getWidth() * (float) v[(size_t) s])), 2.5f);
        hits_.push_back({ track, {}, [this, s](double x) { act([=](Studio& st) { st.drums.setValue(s, x); }); } });
        if (info(s).bred) {
          const bool locked = view_.locks[(size_t) s];
          button(g, lock, locked ? "Locked" : "Lock", locked, true, [this, s, locked] { act([=](Studio& st) { st.drums.setLock(s, !locked); }); });
        }
      }
      col.removeFromTop(2);
    }
  }
}

void BloodlineBreaks2Editor::paintPads(juce::Graphics& g, juce::Rectangle<int> area) {
  auto head = area.removeFromTop(16);
  label(g, head.removeFromLeft(60), "KEEP");
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawText("drag a drum here (again for a sibling); the loops on the right play only what you keep", head, juce::Justification::centredRight);
  area.removeFromTop(4);
  padAreas_.assign(Drums::kPads, {});
  const int w = (area.getWidth() - 7 * 6) / Drums::kPads;
  for (int p = 0; p < Drums::kPads; ++p) {
    const auto r = juce::Rectangle<int>(area.getX() + p * (w + 6), area.getY(), w, area.getHeight());
    padAreas_[(size_t) p] = r;
    const Pad& pad = view_.pads[(size_t) p];
    const bool target = carry_ && carry_->moving && r.contains(carry_->at) &&
                        (carry_->what == Carryable::DrumTile || (carry_->what == Carryable::DrumPad && carry_->index != p));
    const float flash = juce::jmax(0.0f, 1.0f - view_.padFlash[(size_t) p] / 8.0f);
    g.setColour((pad.full ? kTile : kPanel).interpolatedWith(kKeep, flash * 0.35f + (target ? 0.25f : 0.0f)));
    g.fillRoundedRectangle(r.toFloat(), 6.0f);
    g.setColour(target ? kKeep : pad.full ? zoneColour(pad.zone).withAlpha(0.7f) : kEdge);
    g.drawRoundedRectangle(r.toFloat().reduced(0.5f), 6.0f, target ? 2.0f : 1.0f);
    auto in = r.reduced(5, 4);
    auto top = in.removeFromTop(14);
    g.setColour(kMuted);
    g.setFont(mono(10.0f));
    g.drawText(juce::String(kFirstPadNote + p), top, juce::Justification::centredLeft);
    if (!pad.full) {
      g.drawText("drop", in, juce::Justification::centred);
      continue;
    }
    g.setColour(zoneColour(pad.zone));
    g.setFont(font(11.0f, true));
    g.drawFittedText(pad.name, in.removeFromTop(16), juce::Justification::centredLeft, 1, 0.7f);
    paintSound(g, pad.sound, in.withTrimmedBottom(2), 0.8f);
    Hit h{ r, [this, p] { proc_.playPad(p); } };
    h.carry = Carryable::DrumPad;
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

void BloodlineBreaks2Editor::paintLoops(juce::Graphics& g, juce::Rectangle<int> area) {
  const int x = area.getX(), w = area.getWidth();
  // [style][back][new]
  label(g, { x, kRow1, 44, 26 }, "STYLE");
  for (int s = 0; s < kNumStyles; ++s)
    button(g, { x + 44 + s * 64, kRow1, 64, 26 }, styleInfo((Style) s).name, (int) view_.ops.style == s, true,
           [this, s] { act([s](Studio& st) { st.loops.setStyle((Style) s); }); });
  button(g, { x + w - 140, kRow1, 64, 26 }, "Back", false, view_.loopBack, [this] { act([](Studio& s) { s.loops.back(); }); });
  button(g, { x + w - 72, kRow1, 72, 26 }, "New", false, true, [this] { act([](Studio& s) { s.loops.fresh(); }); });

  // [playing][midi][swing][pull][spread]
  {
    if (view_.play) midiNote_ = false;
    button(g, { x, kRow2, 64, 26 }, view_.play ? "Playing" : "Play", view_.play, true,
           [this] { proc_.setPlay(!view_.play); timerCallback(); });
    auto drag = juce::Rectangle<int>(x + 68, kRow2, 88, 26);
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
    if (midiNote_ && !view_.play) {
      g.setColour(kParent);
      g.setFont(font(10.0f));
      g.drawText("Playing off: the clip plays the kit", x, kRow2 + 26, 240, 13, juce::Justification::centredLeft);
    }

    label(g, { x + 162, kRow2, 40, 26 }, "SWING");
    slider(g, { x + 202, kRow2, 62, 26 }, (view_.ops.swingDial - kSwingMin) / (kSwingMax - kSwingMin), kAccent,
           [this](double v) { act([v](Studio& s) { s.loops.setSwingDial(kSwingMin + v * (kSwingMax - kSwingMin)); }); });
    g.setColour(kFg);
    g.setFont(mono(11.0f));
    g.drawText(pct(view_.ops.swingDial), x + 268, kRow2, 32, 26, juce::Justification::centredLeft);
    label(g, { x + 304, kRow2, 32, 26 }, "PULL");
    slider(g, { x + 336, kRow2, 54, 26 }, view_.ops.pull, kParent, [this](double v) { act([v](Studio& s) { s.loops.setPull(v); }); });
    g.setColour(kFg);
    g.setFont(mono(11.0f));
    g.drawText(pullName(view_.ops.pull), x + 394, kRow2, 38, 26, juce::Justification::centredLeft);
    spreadSlider(g, { x + 436, kRow2, w - 436, 26 }, view_.loopSpread, [this](double s) { act([s](Studio& st) { st.loops.setSpread(s); }); });
  }

  // [A][B][C][D]
  loopLitter_ = { x, kTilesY, w, kTileH };
  const int tw = (w - 3 * 8) / Loops::kLitter;
  for (int i = 0; i < (int) view_.loops.size(); ++i) paintLoopTile(g, i, { x + i * (tw + 8), kTilesY, tw, kTileH });

  paintLoopSettings(g, { x, kSettingsY, w, kSettingsH });
  paintSlots(g, { x, kKeepY, w, kKeepH });
}

// Lanes bottom up: kick at the bottom, then snare, hat, and perc on top. Hits are drawn where they
// land once swung; the sixteenths are only a guide. Locked lanes get a faint band.
void BloodlineBreaks2Editor::paintGrid(juce::Graphics& g, const Pattern& p, juce::Rectangle<int> area, bool live, bool big, bool locks) {
  const juce::Graphics::ScopedSaveState keepClip(g);
  g.reduceClipRegion(area);
  const auto a = area.toFloat();
  g.setColour(kBg.withAlpha(0.55f));
  g.fillRoundedRectangle(a, 4.0f);
  const float cw = a.getWidth() / kSteps, rh = a.getHeight() / kLanes;
  for (int s = 0; s < kSteps; s += 4) {
    g.setColour(kEdge.withAlpha(0.35f));
    g.fillRect(a.getX() + s * cw, a.getY(), cw, a.getHeight());
  }
  if (big)
    for (int s = 1; s < kSteps; ++s) {
      g.setColour(kEdge.withAlpha(s % 4 == 0 ? 0.6f : 0.3f));
      g.fillRect(a.getX() + s * cw, a.getY(), 1.0f, a.getHeight());
    }
  if (locks)
    for (int l = 0; l < kLanes; ++l)
      if (view_.ops.locks[(size_t) l]) {
        g.setColour(kLock.withAlpha(0.16f));
        g.fillRect(a.getX(), a.getY() + (kLanes - 1 - l) * rh, a.getWidth(), rh);
      }
  if (live && view_.running) {
    const float px = a.getX() + (float) (view_.beat / 4.0) * a.getWidth();
    g.setColour(kFg.withAlpha(0.6f));
    g.fillRect(px, a.getY(), 1.5f, a.getHeight());
  }
  for (int l = 0; l < kLanes; ++l) {
    const float rowY = a.getY() + (kLanes - 1 - l) * rh;
    const juce::Colour c = kLaneColour[l];
    const LaneData& lane = p.lanes[(size_t) l];
    const float bw = big ? juce::jmin(cw - 2.0f, 12.0f) : juce::jmax(2.0f, cw - 1.0f);
    for (int i = 0; i < lane.count; ++i) {
      const Stroke& h = lane.hits[(size_t) i];
      const float bx = a.getX() + (float) warp(h.pos, p.swing) * cw;
      const float hh = (rh - 3.0f) * (big ? h.vel : 0.3f + 0.7f * h.vel);
      const float y = big ? rowY + rh - 1.5f - hh : rowY + (rh - hh) / 2.0f;
      const int rolls = juce::jlimit(1, 4, (int) h.roll);
      g.setColour(c.withAlpha(live ? 1.0f : 0.8f).withMultipliedBrightness(0.6f + 0.4f * h.vel));
      if (rolls == 1) g.fillRoundedRectangle(bx + 0.5f, y, bw, hh, big ? 2.0f : 1.0f);
      else
        for (int r = 0; r < rolls; ++r) g.fillRect(bx + 0.5f + r * cw / rolls, y, juce::jmax(1.0f, cw / rolls - 1.0f), hh);
    }
  }
}

void BloodlineBreaks2Editor::paintLoopTile(juce::Graphics& g, int i, juce::Rectangle<int> area) {
  const Pattern& p = view_.loops[(size_t) i];
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
  else if (live) { g.setColour(kAccent); g.drawText(view_.play ? "playing" : "heard", top, juce::Justification::centredRight); }
  in.removeFromTop(2);
  paintGrid(g, p, in.removeFromTop(74), live, false, true);
  in.removeFromTop(2);
  g.setColour(kMuted);
  g.setFont(mono(10.0f));
  g.drawText("swing " + pct(p.swing) + "  " + juce::String(p.hits()) + " hits", in.removeFromTop(16), juce::Justification::centredLeft);

  auto actions = area.reduced(6, 5).removeFromBottom(26);
  const int half = actions.getWidth() / 2;
  button(g, actions.removeFromLeft(half), parent ? "Again" : "Breed", false, true, [this, i] { act([i](Studio& s) { s.loops.breedFrom(i); }); });
  button(g, actions, "Cross", false, !heard, [this, i] { act([i](Studio& s) { s.loops.cross(s.loops.litter().selected(), i); }); });
}

// [first sound | second sound][One/Stack/Flam][Lock]. A kept pad dropped on either half sets it.
void BloodlineBreaks2Editor::paintLaneHead(juce::Graphics& g, int l, juce::Rectangle<int> h) {
  const LaneData& lane = view_.playing.lanes[(size_t) l];
  const bool locked = view_.ops.locks[(size_t) l];
  g.setColour(kLaneColour[l]);
  g.fillRoundedRectangle(h.removeFromLeft(5).toFloat(), 2.0f);
  h.removeFromLeft(5);
  auto lock = h.removeFromRight(40);
  h.removeFromRight(3);
  auto layer = h.removeFromRight(46);
  h.removeFromRight(3);
  auto first = h.removeFromLeft(h.getWidth() * 55 / 100), second = h;
  soundAreas_[(size_t) l] = { first, second };

  const bool carryingPad = carry_ && carry_->moving && carry_->what == Carryable::DrumPad;
  auto half = [&](juce::Rectangle<int> r, bool left, const juce::String& text, bool empty) {
    const bool target = carryingPad && r.contains(carry_->at);
    const auto f = r.toFloat().reduced(1.0f, 1.5f);
    g.setColour(target ? kKeep.withAlpha(0.35f) : empty ? kPanel : kTile);
    if (left) g.fillRoundedRectangle(f.withTrimmedRight(-4.0f), 4.0f);
    else g.fillRoundedRectangle(f.withTrimmedLeft(-4.0f), 4.0f);
    g.setColour(target ? kKeep : kEdge);
    g.drawRoundedRectangle(f.reduced(0.5f), 4.0f, 1.0f);
    g.setColour(empty ? kMuted : kFg);
    g.setFont(font(11.0f, !empty));
    g.drawFittedText(text, r.reduced(4, 0), juce::Justification::centredLeft, 1, 0.7f);
  };
  half(first, true, padName(lane.first), lane.first < 0 || !view_.pads[(size_t) lane.first].full);
  hits_.push_back({ first, [this, l] { act([l](Studio& s) { s.loops.cycleSound(l); }); } });

  const bool hasSecond = lane.second >= 0;
  if (!hasSecond) {
    half(second, false, "+", true);
    hits_.push_back({ second, [this, l] { act([l](Studio& s) { s.loops.cycleSecond(l); }); } });
  } else {
    half(second.withTrimmedRight(14), false, padName(lane.second), !view_.pads[(size_t) lane.second].full);
    hits_.push_back({ second.withTrimmedRight(14), [this, l] { act([l](Studio& s) { s.loops.cycleSecond(l); }); } });
    const auto x = second.removeFromRight(14);
    g.setColour(kMuted);
    g.setFont(font(12.0f, true));
    g.drawText(juce::String::fromUTF8("\xc3\x97"), x, juce::Justification::centred);
    hits_.push_back({ x, [this, l] { act([l](Studio& s) { s.loops.setSound(l, 1, -1); }); } });
  }

  // The layer: click cycles One, Stack, Flam; in Flam, drag sideways for the ms.
  const int layerNow = hasSecond ? (int) lane.layer : One;
  const juce::String text = layerNow == Flam ? juce::String(juce::roundToInt(lane.flam)) + "ms" : juce::String(kLayerName[layerNow]);
  button(g, layer, text, layerNow != One, true, {}, kParent);
  hits_.pop_back();
  Hit lh{ layer, [this, l] { act([l](Studio& s) { s.loops.cycleLayer(l); }); } };
  if (layerNow == Flam) {
    const float start = lane.flam;
    lh.move = [this, l, start](juce::Point<int> d) { act([=](Studio& s) { s.loops.setFlam(l, start + d.x * 0.25); }); };
  }
  hits_.push_back(lh);
  button(g, lock, locked ? "Held" : "Lock", locked, true, [this, l, locked] { act([=](Studio& s) { s.loops.setLock(l, !locked); }); }, kLock);
}

void BloodlineBreaks2Editor::paintLoopSettings(juce::Graphics& g, juce::Rectangle<int> area) {
  g.setColour(kPanel);
  g.fillRoundedRectangle(area.toFloat(), 8.0f);
  auto in = area.reduced(12, 8);
  const Pattern& p = view_.playing;
  const bool fromSlot = view_.playingSlot >= 0;
  auto head = in.removeFromTop(20);
  g.setColour(kFg);
  g.setFont(font(14.0f, true));
  g.drawText(fromSlot ? "KEPT LOOP " + juce::String(view_.playingSlot + 1)
                      : "LOOP " + letter(view_.loopSelected) + (view_.loopSelected == view_.loopParent ? "  " + star() : juce::String()),
             head.removeFromLeft(130), juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawText("Click to add a hit, drag to move it (Shift snaps), up and down for velocity, click it to remove.", head,
             juce::Justification::centredRight);
  in.removeFromTop(4);

  auto counts = in.removeFromTop(14).withTrimmedLeft(kHead);
  const float cw = counts.getWidth() / (float) kSteps;
  const char* names[4] = { "", "e", "&", "a" };
  g.setFont(mono(10.0f));
  for (int s = 0; s < kSteps; ++s) {
    g.setColour(s % 4 == 0 ? kFg : kMuted);
    g.drawText(s % 4 == 0 ? juce::String(s / 4 + 1) : juce::String(names[s % 4]), (int) (counts.getX() + s * cw), counts.getY(), (int) cw, 14,
               juce::Justification::centred);
  }

  auto lanes = in.removeFromTop(kLanes * kLaneH);
  bigGrid_ = lanes.withTrimmedLeft(kHead);
  for (int row = 0; row < kLanes; ++row) {
    const int l = kLanes - 1 - row; // perc on top, kick at the bottom
    auto r = lanes.removeFromTop(kLaneH);
    if (view_.ops.locks[(size_t) l]) {
      g.setColour(kLock.withAlpha(0.1f));
      g.fillRoundedRectangle(r.removeFromLeft(kHead).toFloat().reduced(0, 2).withTrimmedRight(4.0f), 4.0f);
    }
    paintLaneHead(g, l, juce::Rectangle<int>(in.getX(), r.getY(), kHead - 6, kLaneH).reduced(0, 5));
  }
  paintGrid(g, p, bigGrid_, true, true, true);

  in.removeFromTop(8);
  auto sw = in.removeFromTop(24);
  label(g, sw.removeFromLeft(kHead), "THIS LOOP'S SWING");
  g.setColour(kFg);
  g.setFont(mono(11.0f));
  g.drawText(pct(p.swing), sw.removeFromRight(40), juce::Justification::centredRight);
  slider(g, sw.withTrimmedRight(8), (p.swing - kSwingMin) / (kSwingMax - kSwingMin), kAccent,
         [this](double v) { act([v](Studio& s) { s.loops.setSwing(kSwingMin + v * (kSwingMax - kSwingMin)); }); });

  in.removeFromTop(4);
  if (!view_.anyKept) {
    g.setColour(kParent);
    g.setFont(font(13.0f, true));
    g.drawFittedText("Keep a drum on the left to hear the loops.", in, juce::Justification::centred, 2);
    return;
  }
  const auto& st = styleInfo(view_.ops.style);
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  const juce::String styleText = view_.ops.style == Style::None
                                     ? juce::String("No style: flat odds, no lean. Swing and pull steer the next breed or New. ")
                                     : juce::String(st.name) + " sits around " + juce::String(st.bpm) + " BPM; Live is at " +
                                           juce::String(juce::roundToInt(view_.bpm)) + ". Style, swing and pull steer the next breed or New, " +
                                           pullName(view_.ops.pull) + "ly. ";
  g.drawFittedText(styleText + "Each lane plays kept drums with its label, then Drum pads; drag a pad onto a sound to choose it.", in,
                   juce::Justification::topLeft, 3);
}

void BloodlineBreaks2Editor::paintSlots(juce::Graphics& g, juce::Rectangle<int> area) {
  auto head = area.removeFromTop(16);
  label(g, head.removeFromLeft(60), "KEEP");
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawText("drag a loop here; click to play it, or play 48 to 55", head, juce::Justification::centredRight);
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
    g.drawText(juce::String(s + 1) + "  " + juce::String(kFirstSlotNote + s), top, juce::Justification::centredLeft);
    if (!slot.full) {
      g.drawText("drop", in, juce::Justification::centred);
      continue;
    }
    in.removeFromTop(4);
    paintGrid(g, slot.loop, in.withTrimmedBottom(2), live, false, false);
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

void BloodlineBreaks2Editor::paintCarry(juce::Graphics& g) {
  if (!carry_ || !carry_->moving) return;
  const auto what = carry_->what;
  const int i = carry_->index;
  juce::String text;
  if (what == Carryable::DrumTile || what == Carryable::LoopTile) text = letter(i);
  else if (what == Carryable::DrumPad) text = view_.pads[(size_t) i].name;
  else text = "Kept " + juce::String(i + 1);
  const juce::Rectangle<int> home = what == Carryable::DrumPad ? drumLitter_ : what == Carryable::LoopSlot ? loopLitter_ : juce::Rectangle<int>();
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

int BloodlineBreaks2Editor::indexAt(const std::vector<juce::Rectangle<int>>& areas, juce::Point<int> p) const {
  for (int i = 0; i < (int) areas.size(); ++i)
    if (areas[(size_t) i].contains(p)) return i;
  return -1;
}

// The big grid: press a hit to drag it (or click it away), press empty space to add one.
bool BloodlineBreaks2Editor::gridDown(juce::Point<int> pos) {
  if (!bigGrid_.contains(pos)) return false;
  const auto a = bigGrid_.toFloat();
  const float cw = a.getWidth() / kSteps, rh = a.getHeight() / kLanes;
  const int row = juce::jlimit(0, kLanes - 1, (int) ((pos.y - a.getY()) / rh));
  const int l = kLanes - 1 - row;
  const Pattern& p = view_.playing;
  const LaneData& lane = p.lanes[(size_t) l];
  const double landed = (pos.x - a.getX()) / cw;
  const float bw = juce::jmin(cw - 2.0f, 12.0f);
  int best = -1;
  double bestD = 1e9;
  for (int i = 0; i < lane.count; ++i) {
    const double hx = warp(lane.hits[(size_t) i].pos, p.swing);
    const double px = (landed - hx) * cw; // pixels from the hit's left edge
    if (px < -3.0 || px > bw + 3.0) continue;
    const double d = std::abs(px - bw / 2.0);
    if (d < bestD) { bestD = d; best = i; }
  }
  if (best >= 0) {
    gridDrag_ = GridDrag{ l, best, warp(lane.hits[(size_t) best].pos, p.swing), lane.hits[(size_t) best].vel, true, false };
    return true;
  }
  const int step = (int) std::lround(unwarp(landed, p.swing)) % kSteps;
  int index = -1;
  act([&](Studio& s) { index = s.loops.addHit(l, step, 0.9f); });
  if (index >= 0) gridDrag_ = GridDrag{ l, index, warp(step, p.swing), 0.9f, false, false };
  return true;
}

void BloodlineBreaks2Editor::mouseDown(const juce::MouseEvent& e) {
  const auto pos = e.getPosition();
  carry_.reset();
  gridDrag_.reset();
  dragging_.reset();
  midiDragStarted_ = false;
  for (auto it = hits_.rbegin(); it != hits_.rend(); ++it) {
    if (!it->area.contains(pos)) continue;
    Hit h = *it;
    if (h.midiOut || h.move) { dragging_ = h; return; }
    if (h.drag) { dragging_ = h; h.drag(juce::jlimit(0.0, 1.0, (pos.x - h.area.getX()) / (double) juce::jmax(1, h.area.getWidth()))); return; }
    if (h.carry != Carryable::None) carry_ = Carry{ h.carry, h.index, pos };
    if (h.click) h.click();
    return;
  }
  gridDown(pos);
}

void BloodlineBreaks2Editor::mouseDrag(const juce::MouseEvent& e) {
  if (gridDrag_) {
    auto& d = *gridDrag_;
    const auto off = e.getOffsetFromDragStart();
    if (!d.moved && off.getDistanceFromOrigin() < 3) return;
    d.moved = true;
    const auto a = bigGrid_.toFloat();
    const float cw = a.getWidth() / kSteps, rh = a.getHeight() / kLanes;
    const double swing = view_.playing.swing;
    double pos = unwarp(wrapBar(d.startLanded + off.x / cw), swing);
    if (e.mods.isShiftDown()) pos = wrapBar(std::round(pos));
    const float vel = juce::jlimit(0.08f, 1.0f, d.startVel - off.y / (rh - 3.0f));
    const int lane = d.lane, index = d.index;
    act([=](Studio& s) { s.loops.moveHit(lane, index, pos, vel); });
    return;
  }
  if (carry_) {
    carry_->at = e.getPosition();
    if (e.getDistanceFromDragStart() > 6) carry_->moving = true;
    repaint();
    return;
  }
  if (!dragging_) return;
  if (dragging_->midiOut) {
    if (midiDragStarted_ || e.getDistanceFromDragStart() < 6) return;
    midiDragStarted_ = true;
    // The clip takes over: Playing goes off so the loop does not double it.
    proc_.setPlay(false);
    midiNote_ = true;
    timerCallback();
    const juce::File f = proc_.writeMidiFile();
    if (f.existsAsFile()) juce::DragAndDropContainer::performExternalDragDropOfFiles({ f.getFullPathName() }, false, this);
    return;
  }
  if (dragging_->move) {
    if (e.getDistanceFromDragStart() < 3 && !midiDragStarted_) return;
    midiDragStarted_ = true; // here: moved, so no click on release
    dragging_->move(e.getOffsetFromDragStart());
    return;
  }
  const auto& a = dragging_->area;
  dragging_->drag(juce::jlimit(0.0, 1.0, (e.getPosition().x - a.getX()) / (double) juce::jmax(1, a.getWidth())));
}

void BloodlineBreaks2Editor::mouseUp(const juce::MouseEvent& e) {
  if (gridDrag_) {
    const GridDrag d = *gridDrag_;
    gridDrag_.reset();
    if (d.existed && !d.moved) act([=](Studio& s) { s.loops.removeHit(d.lane, d.index); });
    else act([=](Studio& s) { s.loops.tidy(d.lane); });
    return;
  }
  if (dragging_ && dragging_->move && !midiDragStarted_ && dragging_->click) dragging_->click();
  dragging_.reset();
  if (!carry_ || !carry_->moving) { carry_.reset(); return; }
  const Carry c = *carry_;
  carry_.reset();
  const auto at = e.getPosition();
  const int pad = indexAt(padAreas_, at), slot = indexAt(slotAreas_, at);
  const int i = c.index;
  switch (c.what) {
    case Carryable::DrumTile:
      if (pad >= 0) act([=](Studio& s) { s.keepDrum(i, pad); });
      break;
    case Carryable::DrumPad: {
      int lane = -1, which = -1;
      for (int l = 0; l < kLanes; ++l)
        for (int k = 0; k < 2; ++k)
          if (soundAreas_[(size_t) l][(size_t) k].contains(at)) { lane = l; which = k; }
      if (lane >= 0) act([=](Studio& s) { s.loops.setSound(lane, which, i); });
      else if (pad >= 0 && pad != i) act([=](Studio& s) { s.swapPads(i, pad); });
      else if (drumLitter_.contains(at)) act([=](Studio& s) { s.drums.breedFromPad(i); });
      break;
    }
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
