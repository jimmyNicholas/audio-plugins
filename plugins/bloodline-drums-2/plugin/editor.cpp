#include "editor.h"
#include <cmath>

using namespace drums2;
using namespace bloodline;

namespace {

juce::String num(double v, int places) { return places == 0 ? juce::String(juce::roundToInt(v)) : juce::String(v, places); }

const juce::Colour kBg(0xff15171c), kPanel(0xff1f2229), kTile(0xff292d36), kEdge(0xff3a3f4b);
const juce::Colour kFg(0xffe8e6e1), kMuted(0xff8d8f96);
const juce::Colour kAccent(0xffff6b6b), kParent(0xffffc857), kKeep(0xff9be564);

const juce::Colour kGroupColour[kNumGroups] = {
  juce::Colour(0xff7cc6fe), juce::Colour(0xffff8c69), juce::Colour(0xff9be564), juce::Colour(0xffd59bf6), juce::Colour(0xfff4d35e),
};

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

juce::Font font(float size, bool bold = false) { return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain)); }
juce::Font mono(float size) { return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain)); }
juce::String letter(int i) { return juce::String::charToString((juce::juce_wchar) ('A' + i)); }

constexpr int kWidth = 1080, kHeight = 720;
const Zone kZones[kNumZones] = { Zone::None, Zone::Kick, Zone::Snare, Zone::Hats };

} // namespace

BloodlineDrums2Editor::BloodlineDrums2Editor(BloodlineDrums2Processor& p) : AudioProcessorEditor(&p), proc_(p), view_(p.view()) {
  setSize(kWidth, kHeight);
  startTimerHz(30);
}

BloodlineDrums2Editor::~BloodlineDrums2Editor() { stopTimer(); }

void BloodlineDrums2Editor::timerCallback() {
  view_ = proc_.view();
  repaint();
}

void BloodlineDrums2Editor::act(const std::function<void(Kit&)>& change) {
  proc_.edit(change);
  timerCallback();
}

void BloodlineDrums2Editor::button(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled,
                                   std::function<void()> click) {
  const auto f = r.toFloat().reduced(1.5f);
  const float a = enabled ? 1.0f : 0.35f;
  g.setColour((on ? kAccent : kTile).withMultipliedAlpha(a));
  g.fillRoundedRectangle(f, 4.0f);
  g.setColour((on ? kAccent : kEdge).withMultipliedAlpha(a));
  g.drawRoundedRectangle(f.reduced(0.5f), 4.0f, 1.0f);
  g.setColour((on ? kBg : kFg).withMultipliedAlpha(a));
  g.setFont(font(12.0f, on));
  g.drawFittedText(text, r.reduced(2, 0), juce::Justification::centred, 1, 0.8f);
  if (enabled) hits_.push_back({ r, std::move(click), {} });
}

void BloodlineDrums2Editor::paint(juce::Graphics& g) {
  hits_.clear();
  g.fillAll(kBg);
  g.setColour(kFg);
  g.setFont(font(18.0f, true));
  g.drawText("BLOODLINE DRUMS 2", 16, 10, 220, 28, juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(mono(11.0f));
  g.drawText("prototype  |  click to hear, Breed to breed, drag a tile onto a pad to keep it and start the next drum", 220, 10, 700, 28,
             juce::Justification::centredLeft);

  g.setColour(kMuted);
  g.setFont(font(11.0f, true));
  g.drawText("ZONE", 16, 46, 44, 26, juce::Justification::centredLeft);
  for (int z = 0; z < kNumZones; ++z)
    button(g, { 60 + z * 70, 46, 70, 26 }, zoneName(kZones[z]), view_.zone == kZones[z], true,
           [this, z] { act([z](Kit& k) { k.setZone(kZones[z]); }); });
  button(g, { 470, 46, 64, 26 }, "Back", false, view_.canGoBack, [this] { act([](Kit& k) { k.back(); }); });
  button(g, { 536, 46, 100, 26 }, "Fresh litter", false, true, [this] { act([](Kit& k) { k.fresh(); }); });

  g.setColour(kMuted);
  g.drawText("LOOP", 16, 80, 44, 26, juce::Justification::centredLeft);
  button(g, { 60, 80, 64, 26 }, view_.loopOn ? "On" : "Off", view_.loopOn, true, [this] { proc_.setLoop(!view_.loopOn, view_.pattern); timerCallback(); });
  for (int p = 0; p < Loop::kPatterns; ++p)
    button(g, { 132 + p * 76, 80, 76, 26 }, Loop::patternName(p), view_.pattern == p, true,
           [this, p] { proc_.setLoop(view_.loopOn, p); timerCallback(); });
  {
    auto r = juce::Rectangle<int>(372, 80, 264, 26);
    g.setColour(kMuted);
    g.setFont(mono(11.0f));
    g.drawText("SPREAD", r.removeFromLeft(56), juce::Justification::centredLeft);
    auto meter = r.removeFromLeft(90).withSizeKeepingCentre(90, 6).toFloat();
    g.setColour(kTile);
    g.fillRoundedRectangle(meter, 3.0f);
    g.setColour(kAccent);
    g.fillRoundedRectangle(meter.withWidth(juce::jmax(3.0f, meter.getWidth() * (float) ((view_.spread - 0.02) / 0.48))), 3.0f);
    r.removeFromLeft(8);
    g.setColour(kMuted);
    g.drawText(juce::String(view_.spread, 2) + "  gen " + juce::String(view_.generation), r, juce::Justification::centredLeft);
  }

  litterArea_ = { 16, 118, 620, 452 };
  for (int i = 0; i < (int) view_.candidates.size(); ++i) {
    const int col = i % 4, row = i / 4;
    paintTile(g, i, { 16 + col * 157, 118 + row * 230, 149, 222 });
  }
  paintPads(g, { 16, 582, 620, 122 });
  paintInspector(g, { 652, 46, kWidth - 652 - 16, kHeight - 46 - 16 });

  // What is being carried, under the mouse.
  if (carry_ && carry_->moving) {
    const bool fromPad = carry_->pad >= 0;
    const juce::String label = fromPad ? juce::String(view_.pads[(size_t) carry_->pad].name) : letter(carry_->tile);
    if (fromPad && litterArea_.contains(carry_->at)) {
      g.setColour(kParent.withAlpha(0.12f));
      g.fillRoundedRectangle(litterArea_.toFloat(), 8.0f);
      g.setColour(kParent);
      g.setFont(font(14.0f, true));
      g.drawText("drop to breed from " + label, litterArea_.withHeight(30).translated(0, -2), juce::Justification::centredTop);
    }
    auto ghost = juce::Rectangle<float>(0, 0, 90, 40).withCentre(carry_->at.toFloat());
    g.setColour(kKeep.withAlpha(0.85f));
    g.fillRoundedRectangle(ghost, 6.0f);
    g.setColour(kBg);
    g.setFont(font(13.0f, true));
    g.drawFittedText(label, ghost.toNearestInt(), juce::Justification::centred, 1);
  }
}

// The amplitude of body and noise over the first half second, and a bar per setting.
void BloodlineDrums2Editor::paintSound(juce::Graphics& g, const Values& v, juce::Rectangle<int> area, float alpha) {
  auto env = area.removeFromTop(area.getHeight() / 2).toFloat().reduced(0, 3);
  g.setColour(kBg.withAlpha(0.5f));
  g.fillRoundedRectangle(env, 4.0f);
  const double decay = toReal(Decay, v[Decay]), noiseDecay = toReal(NoiseDecay, v[NoiseDecay]), mix = v[BodyNoise];
  juce::Path body, noise;
  for (int x = 0; x <= (int) env.getWidth(); ++x) {
    const double t = 0.5 * x / env.getWidth();
    const float yb = env.getBottom() - (float) ((1.0 - mix) * std::pow(0.001, t / decay)) * env.getHeight() * 0.9f;
    const float yn = env.getBottom() - (float) (mix * std::pow(0.001, t / noiseDecay)) * env.getHeight() * 0.9f;
    if (x == 0) { body.startNewSubPath(env.getX(), yb); noise.startNewSubPath(env.getX(), yn); }
    else { body.lineTo(env.getX() + x, yb); noise.lineTo(env.getX() + x, yn); }
  }
  g.setColour(kGroupColour[0].withAlpha(alpha));
  g.strokePath(body, juce::PathStrokeType(1.5f));
  g.setColour(kGroupColour[1].withAlpha(alpha));
  g.strokePath(noise, juce::PathStrokeType(1.5f));

  auto bars = area.toFloat().reduced(0, 4);
  const float w = bars.getWidth() / kNumSettings;
  for (int s = 0; s < kNumSettings; ++s) {
    const float h = juce::jmax(1.0f, (float) v[(size_t) s] * bars.getHeight());
    g.setColour(kGroupColour[groupOf(s)].withAlpha(alpha * 0.85f));
    g.fillRect(bars.getX() + s * w, bars.getBottom() - h, juce::jmax(1.0f, w - 1.0f), h);
  }
}

void BloodlineDrums2Editor::paintTile(juce::Graphics& g, int i, juce::Rectangle<int> area) {
  const Values& v = view_.candidates[(size_t) i];
  const bool heard = i == view_.selected, parent = i == view_.parent;
  const float flash = heard ? juce::jmax(0.0f, 1.0f - view_.tileFlash / 8.0f) : 0.0f;
  const auto rect = area.toFloat();
  g.setColour((heard ? kTile.interpolatedWith(kAccent, 0.2f) : kTile).interpolatedWith(kFg, flash * 0.15f));
  g.fillRoundedRectangle(rect, 8.0f);
  g.setColour(heard ? kAccent : kEdge);
  g.drawRoundedRectangle(rect.reduced(heard ? 1.0f : 0.5f), 8.0f, heard ? 2.0f : 1.0f);
  Hit body{ area.withTrimmedBottom(34), [this, i] { proc_.hearTile(i); timerCallback(); }, {} };
  body.tile = i;
  hits_.push_back(body);

  auto in = area.reduced(10, 8);
  auto top = in.removeFromTop(22);
  g.setColour(kFg);
  g.setFont(font(18.0f, true));
  g.drawText(letter(i), top, juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(mono(10.0f));
  g.drawText(juce::String(Kit::kFirstTile + i), top.withTrimmedLeft(18), juce::Justification::centredLeft);
  g.setFont(font(12.0f, true));
  if (parent) { g.setColour(kParent); g.drawText(juce::String::fromUTF8("\xe2\x98\x85 parent"), top, juce::Justification::centredRight); }
  else if (heard) { g.setColour(kAccent); g.drawText("hearing", top, juce::Justification::centredRight); }

  paintSound(g, v, in.removeFromTop(130), heard ? 1.0f : 0.7f);
  g.setColour(kMuted);
  g.setFont(mono(10.0f));
  g.drawText(formatValue(Pitch, v[Pitch]) + "  " + formatValue(Decay, v[Decay]), in.removeFromTop(18), juce::Justification::centredLeft);

  auto actions = area.reduced(8, 6).removeFromBottom(28);
  const int half = actions.getWidth() / 2;
  button(g, actions.removeFromLeft(half), parent ? "Breed again" : "Breed", false, true, [this, i] { act([i](Kit& k) { k.breedFrom(i); }); });
  button(g, actions, "Cross", false, !heard, [this, i] { act([i](Kit& k) { k.cross(k.litter().selected(), i); }); });
}

void BloodlineDrums2Editor::paintPads(juce::Graphics& g, juce::Rectangle<int> area) {
  g.setColour(kMuted);
  g.setFont(font(11.0f, true));
  g.drawText("KEEP", area.removeFromTop(14), juce::Justification::centredLeft);
  area.removeFromTop(4);
  const int w = (area.getWidth() - 7 * 6) / Kit::kPads;
  for (int p = 0; p < Kit::kPads; ++p) {
    const auto r = juce::Rectangle<int>(area.getX() + p * (w + 6), area.getY(), w, area.getHeight());
    padAreas_[(size_t) p] = r;
    const Pad& pad = view_.pads[(size_t) p];
    const bool target = carry_ && carry_->moving && r.contains(carry_->at) && carry_->pad != p;
    const float flash = juce::jmax(0.0f, 1.0f - view_.padFlash[(size_t) p] / 8.0f);
    g.setColour((pad.full ? kTile : kPanel).interpolatedWith(kKeep, flash * 0.35f + (target ? 0.25f : 0.0f)));
    g.fillRoundedRectangle(r.toFloat(), 6.0f);
    g.setColour(target ? kKeep : pad.full ? kEdge.brighter(0.3f) : kEdge);
    g.drawRoundedRectangle(r.toFloat().reduced(0.5f), 6.0f, target ? 2.0f : 1.0f);
    auto in = r.reduced(6, 4);
    auto top = in.removeFromTop(16);
    g.setColour(kMuted);
    g.setFont(mono(10.0f));
    g.drawText(juce::String(Kit::kFirstPad + p), top, juce::Justification::centredLeft);
    if (!pad.full) {
      g.drawText("drop", in, juce::Justification::centred);
      continue;
    }
    g.setColour(kFg);
    g.setFont(font(11.0f, true));
    g.drawFittedText(pad.name, in.removeFromTop(16), juce::Justification::centredLeft, 1, 0.7f);
    paintSound(g, pad.sound, in.withTrimmedBottom(2), 0.8f);
    Hit h{ r, [this, p] { proc_.playPad(p); }, {} };
    h.pad = p;
    hits_.push_back(h);
    // Clear, top right.
    const auto x = juce::Rectangle<int>(r.getRight() - 18, r.getY() + 3, 15, 15);
    g.setColour(kMuted);
    g.setFont(font(12.0f, true));
    g.drawText(juce::String::fromUTF8("\xc3\x97"), x, juce::Justification::centred);
    hits_.push_back({ x, [this, p] { act([p](Kit& k) { k.clearPad(p); }); }, {} });
  }
}

void BloodlineDrums2Editor::paintInspector(juce::Graphics& g, juce::Rectangle<int> area) {
  g.setColour(kPanel);
  g.fillRoundedRectangle(area.toFloat(), 8.0f);
  auto in = area.reduced(14, 10);
  const Values& v = view_.candidates[(size_t) view_.selected];
  g.setColour(kFg);
  g.setFont(font(15.0f, true));
  g.drawText("HEARING " + letter(view_.selected) + (view_.selected == view_.parent ? juce::String::fromUTF8("  \xe2\x98\x85") : juce::String()),
             in.removeFromTop(24), juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawText("Drag to change it. Locks keep a setting when breeding.", in.removeFromTop(16), juce::Justification::centredLeft);
  in.removeFromTop(6);

  for (int gi = 0; gi < kNumGroups; ++gi) {
    const Group& grp = group(gi);
    auto head = in.removeFromTop(22);
    g.setColour(kGroupColour[gi]);
    g.setFont(font(12.0f, true));
    g.drawText(juce::String(grp.name).toUpperCase(), head, juce::Justification::centredLeft);
    for (int s = grp.first; s < grp.first + grp.count; ++s) {
      auto r = in.removeFromTop(22);
      g.setColour(kMuted);
      g.setFont(font(12.0f));
      g.drawText(info(s).name, r.removeFromLeft(92), juce::Justification::centredLeft);
      g.setColour(kFg);
      g.setFont(mono(11.0f));
      g.drawText(formatValue(s, v[(size_t) s]), r.removeFromLeft(72), juce::Justification::centredLeft);
      auto lock = r.removeFromRight(52);
      r.removeFromRight(8);
      const auto track = r.reduced(0, 3);
      const auto bar = track.toFloat().withSizeKeepingCentre((float) track.getWidth(), 5.0f);
      g.setColour(kTile);
      g.fillRoundedRectangle(bar, 2.5f);
      if (view_.zone != Zone::None) {
        const Range z = zoneRange(view_.zone, s);
        if (z.hi - z.lo < 1.0) {
          g.setColour(kGroupColour[gi].withAlpha(0.3f));
          g.fillRect(bar.getX() + bar.getWidth() * (float) z.lo, bar.getBottom() + 2.0f, bar.getWidth() * (float) (z.hi - z.lo), 2.0f);
        }
      }
      g.setColour(kGroupColour[gi]);
      g.fillRoundedRectangle(bar.withWidth(juce::jmax(2.0f, bar.getWidth() * (float) v[(size_t) s])), 2.5f);
      hits_.push_back({ track, {}, [this, s](double x) { act([=](Kit& k) { k.setValue(s, x); }); } });
      if (info(s).bred) {
        const bool locked = view_.locks[(size_t) s];
        button(g, lock, locked ? "Locked" : "Lock", locked, true, [this, s, locked] { act([=](Kit& k) { k.setLock(s, !locked); }); });
      }
    }
    in.removeFromTop(2);
  }
  in.removeFromTop(6);
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawFittedText("MPD226 Bank A: the top two rows (44 to 51) pick and play A to H; the bottom two (36 to 43) play the pads. "
                   "A clip on 36 to 43 plays the kit.",
                   in, juce::Justification::topLeft, 4);
}

int BloodlineDrums2Editor::padAt(juce::Point<int> p) const {
  for (int i = 0; i < Kit::kPads; ++i)
    if (padAreas_[(size_t) i].contains(p)) return i;
  return -1;
}

void BloodlineDrums2Editor::mouseDown(const juce::MouseEvent& e) {
  const auto pos = e.getPosition();
  carry_.reset();
  for (auto it = hits_.rbegin(); it != hits_.rend(); ++it) {
    if (!it->area.contains(pos)) continue;
    Hit h = *it;
    if (h.drag) { dragging_ = h; h.drag(juce::jlimit(0.0, 1.0, (pos.x - h.area.getX()) / (double) juce::jmax(1, h.area.getWidth()))); return; }
    if (h.tile >= 0 || h.pad >= 0) carry_ = Carry{ h.tile, h.pad, pos, pos, false };
    if (h.click) h.click();
    return;
  }
}

void BloodlineDrums2Editor::mouseDrag(const juce::MouseEvent& e) {
  if (carry_) {
    carry_->at = e.getPosition();
    if (e.getDistanceFromDragStart() > 6) carry_->moving = true;
    repaint();
    return;
  }
  if (!dragging_) return;
  const auto& a = dragging_->area;
  dragging_->drag(juce::jlimit(0.0, 1.0, (e.getPosition().x - a.getX()) / (double) juce::jmax(1, a.getWidth())));
}

void BloodlineDrums2Editor::mouseUp(const juce::MouseEvent& e) {
  dragging_.reset();
  if (!carry_ || !carry_->moving) { carry_.reset(); return; }
  const Carry c = *carry_;
  carry_.reset();
  const int pad = padAt(e.getPosition());
  if (c.tile >= 0 && pad >= 0) act([=](Kit& k) { k.keep(c.tile, pad); });
  else if (c.pad >= 0 && pad >= 0 && pad != c.pad) act([=](Kit& k) { k.swapPads(c.pad, pad); });
  else if (c.pad >= 0 && litterArea_.contains(e.getPosition())) act([=](Kit& k) { k.breedFromPad(c.pad); });
  repaint();
}
