#include "editor.h"
#include <cmath>

using namespace synth;

namespace {

// juce::String(x, 0) keeps every decimal, so whole numbers are rounded here.
juce::String num(double v, int places) { return places == 0 ? juce::String(juce::roundToInt(v)) : juce::String(v, places); }

const juce::Colour kBg(0xff15171c), kPanel(0xff1f2229), kTile(0xff292d36), kEdge(0xff3a3f4b);
const juce::Colour kFg(0xffe8e6e1), kMuted(0xff8d8f96);
const juce::Colour kAccent(0xffb48cff), kParent(0xffffc857);

const juce::Colour kGroupColour[kNumGroups] = {
  juce::Colour(0xff7cc6fe), juce::Colour(0xffff8c69), juce::Colour(0xff9be564), juce::Colour(0xffd59bf6), juce::Colour(0xfff4d35e),
};

int groupOf(int s) {
  for (int g = 0; g < kNumGroups; ++g)
    if (s >= group(g).first && s < group(g).first + group(g).count) return g;
  return 0;
}

juce::String shapeName(double v) {
  const char* names[] = { "sine", "tri", "saw", "square" };
  const int k = juce::jlimit(0, 3, (int) std::round(v * 3.0));
  return std::abs(v * 3.0 - k) < 0.15 ? juce::String(names[k]) : juce::String(names[juce::jmin(2, (int) (v * 3.0))]) + ">" + names[juce::jmin(3, (int) (v * 3.0) + 1)];
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

// One cycle of the morphing shape, for drawing (no anti-aliasing needed here).
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

juce::Font font(float size, bool bold = false) { return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain)); }
juce::Font mono(float size) { return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain)); }

constexpr int kWidth = 1080, kHeight = 720;
const Zone kZones[kNumZones] = { Zone::None, Zone::Bass, Zone::Keys, Zone::Lead, Zone::Pad, Zone::Pluck };

} // namespace

SynthBreederEditor::SynthBreederEditor(SynthBreederProcessor& p)
    : AudioProcessorEditor(&p), proc_(p), view_(p.view()), keys_(p.keyboard, juce::MidiKeyboardComponent::horizontalKeyboard) {
  keys_.setAvailableRange(24, 96);
  keys_.setLowestVisibleKey(36);
  keys_.setKeyWidth(18.0f);
  keys_.setMidiChannel(1);
  addAndMakeVisible(keys_);
  setSize(kWidth, kHeight);
  startTimerHz(30);
}

SynthBreederEditor::~SynthBreederEditor() { stopTimer(); }

void SynthBreederEditor::timerCallback() {
  view_ = proc_.view();
  repaint();
}

void SynthBreederEditor::act(const std::function<void(Breeder&)>& change) {
  proc_.edit(change);
  timerCallback();
}

void SynthBreederEditor::resized() { keys_.setBounds(16, 588, 620, 84); }

void SynthBreederEditor::button(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled,
                                std::function<void()> click) {
  const auto f = r.toFloat().reduced(1.5f);
  const float a = enabled ? 1.0f : 0.35f;
  g.setColour((on ? kAccent : kTile).withMultipliedAlpha(a));
  g.fillRoundedRectangle(f, 4.0f);
  g.setColour((on ? kAccent : kEdge).withMultipliedAlpha(a));
  g.drawRoundedRectangle(f.reduced(0.5f), 4.0f, 1.0f);
  g.setColour((on ? kBg : kFg).withMultipliedAlpha(a));
  g.setFont(font(12.0f, on));
  g.drawText(text, r, juce::Justification::centred);
  if (enabled) hits_.push_back({ r, std::move(click), {} });
}

void SynthBreederEditor::paint(juce::Graphics& g) {
  hits_.clear();
  g.fillAll(kBg);
  g.setColour(kFg);
  g.setFont(font(18.0f, true));
  g.drawText("SYNTH BREEDER", 16, 10, 220, 28, juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(mono(11.0f));
  g.drawText("spike  |  click to hear, Breed to breed, Cross to mate with the one you hear", 200, 10, 520, 28,
             juce::Justification::centredLeft);

  // Zone.
  g.setColour(kMuted);
  g.setFont(font(11.0f, true));
  g.drawText("ZONE", 16, 46, 44, 26, juce::Justification::centredLeft);
  for (int z = 0; z < kNumZones; ++z)
    button(g, { 60 + z * 64, 46, 64, 26 }, zoneName(kZones[z]), view_.zone == kZones[z], true,
           [this, z] { act([z](Breeder& b) { b.setZone(kZones[z]); }); });

  // Litter actions and the spread.
  button(g, { 470, 46, 64, 26 }, "Back", false, view_.canGoBack, [this] { act([](Breeder& b) { b.back(); }); });
  button(g, { 536, 46, 100, 26 }, "Fresh litter", false, true, [this] { act([](Breeder& b) { b.fresh(); }); });

  // Riff.
  g.setColour(kMuted);
  g.drawText("RIFF", 16, 80, 44, 26, juce::Justification::centredLeft);
  button(g, { 60, 80, 64, 26 }, view_.riffOn ? "On" : "Off", view_.riffOn, true,
         [this] { proc_.setRiff(!view_.riffOn, view_.pattern); timerCallback(); });
  for (int p = 0; p < Riff::kPatterns; ++p)
    button(g, { 132 + p * 72, 80, 72, 26 }, Riff::patternName(p), view_.pattern == p, true,
           [this, p] { proc_.setRiff(view_.riffOn, p); timerCallback(); });
  {
    auto r = juce::Rectangle<int>(360, 80, 276, 26);
    g.setColour(kMuted);
    g.setFont(mono(11.0f));
    g.drawText("SPREAD", r.removeFromLeft(56), juce::Justification::centredLeft);
    auto meter = r.removeFromLeft(100).withSizeKeepingCentre(100, 6).toFloat();
    g.setColour(kTile);
    g.fillRoundedRectangle(meter, 3.0f);
    g.setColour(kAccent);
    g.fillRoundedRectangle(meter.withWidth(juce::jmax(3.0f, meter.getWidth() * (float) ((view_.spread - 0.02) / 0.48))), 3.0f);
    r.removeFromLeft(8);
    g.setColour(kMuted);
    g.drawText(juce::String(view_.spread, 3) + "   gen " + juce::String(view_.generation), r, juce::Justification::centredLeft);
  }

  for (int i = 0; i < (int) view_.candidates.size(); ++i) {
    const int col = i % 4, row = i / 4;
    paintTile(g, i, { 16 + col * 157, 118 + row * 230, 149, 222 });
  }

  paintInspector(g, { 652, 46, kWidth - 652 - 16, kHeight - 46 - 16 });
}

void SynthBreederEditor::paintTile(juce::Graphics& g, int i, juce::Rectangle<int> area) {
  const Patch& p = view_.candidates[(size_t) i];
  const bool heard = i == view_.selected, parent = i == view_.parent;
  const auto rect = area.toFloat();
  g.setColour(heard ? kTile.interpolatedWith(kAccent, 0.22f) : kTile);
  g.fillRoundedRectangle(rect, 8.0f);
  g.setColour(heard ? kAccent : kEdge);
  g.drawRoundedRectangle(rect.reduced(heard ? 1.0f : 0.5f), 8.0f, heard ? 2.0f : 1.0f);
  hits_.push_back({ area.withTrimmedBottom(34), [this, i] { act([i](Breeder& b) { b.select(i); }); }, {} });

  auto in = area.reduced(10, 8);
  auto top = in.removeFromTop(22);
  g.setColour(kFg);
  g.setFont(font(18.0f, true));
  g.drawText(juce::String::charToString((juce::juce_wchar) ('A' + i)), top, juce::Justification::centredLeft);
  if (parent) {
    g.setColour(kParent);
    g.setFont(font(12.0f, true));
    g.drawText(juce::String::fromUTF8("\xe2\x98\x85 parent"), top, juce::Justification::centredRight);
  } else if (heard) {
    g.setColour(kAccent);
    g.setFont(font(12.0f, true));
    g.drawText("hearing", top, juce::Justification::centredRight);
  }

  // Two cycles of the oscillator mix, and the filter cutoff as a shaded floor.
  auto wave = in.removeFromTop(70).toFloat().reduced(0, 4);
  g.setColour(kBg.withAlpha(0.5f));
  g.fillRoundedRectangle(wave, 4.0f);
  const double mixB = toReal(MixB, p[MixB]), ratio = std::exp2(toReal(TuneB, p[TuneB]) / 12.0);
  juce::Path path;
  for (int x = 0; x <= (int) wave.getWidth(); ++x) {
    const double ph = 2.0 * x / wave.getWidth();
    const double a = shapeAt(ph - std::floor(ph), toReal(ShapeA, p[ShapeA]));
    const double pb = ph * ratio;
    const double b = shapeAt(pb - std::floor(pb), toReal(ShapeB, p[ShapeB]));
    const double y = (1.0 - mixB) * a + mixB * b;
    const float py = wave.getCentreY() - (float) (y * wave.getHeight() * 0.42);
    if (x == 0) path.startNewSubPath(wave.getX() + x, py);
    else path.lineTo(wave.getX() + x, py);
  }
  g.setColour(kAccent.withAlpha(heard ? 1.0f : 0.7f));
  g.strokePath(path, juce::PathStrokeType(1.5f));
  const float cut = (float) p[Cutoff];
  g.setColour(kGroupColour[1].withAlpha(0.18f));
  g.fillRect(wave.withTrimmedTop(wave.getHeight() * (1.0f - (1.0f - cut) * 0.6f)));

  // A bar per setting, coloured by group: the patch at a glance.
  auto bars = in.removeFromTop(64).toFloat().reduced(0, 6);
  const float w = bars.getWidth() / kNumSettings;
  for (int s = 0; s < kNumSettings; ++s) {
    const float h = juce::jmax(1.0f, (float) p[(size_t) s] * bars.getHeight());
    g.setColour(kGroupColour[groupOf(s)].withAlpha(heard ? 0.9f : 0.6f));
    g.fillRect(bars.getX() + s * w, bars.getBottom() - h, juce::jmax(1.0f, w - 1.0f), h);
  }

  g.setColour(kMuted);
  g.setFont(mono(10.0f));
  g.drawText(formatValue(Cutoff, p[Cutoff]) + "  " + formatValue(Attack, p[Attack]), in.removeFromTop(18), juce::Justification::centredLeft);

  auto actions = area.reduced(8, 6).removeFromBottom(28);
  const int half = actions.getWidth() / 2;
  button(g, actions.removeFromLeft(half), parent ? "Breed again" : "Breed", false, true,
         [this, i] { act([i](Breeder& b) { b.breedFrom(i); }); });
  button(g, actions, "Cross", false, !heard, [this, i] {
    act([i](Breeder& b) { b.cross(b.litter().selected(), i); });
  });
}

void SynthBreederEditor::paintInspector(juce::Graphics& g, juce::Rectangle<int> area) {
  g.setColour(kPanel);
  g.fillRoundedRectangle(area.toFloat(), 8.0f);
  auto in = area.reduced(14, 10);
  const Patch& p = view_.candidates[(size_t) view_.selected];
  g.setColour(kFg);
  g.setFont(font(15.0f, true));
  g.drawText("HEARING " + juce::String::charToString((juce::juce_wchar) ('A' + view_.selected)) +
                 (view_.selected == view_.parent ? juce::String::fromUTF8("  \xe2\x98\x85") : juce::String()),
             in.removeFromTop(24), juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawText("Drag to change it. Locks keep a setting when breeding.", in.removeFromTop(16), juce::Justification::centredLeft);
  in.removeFromTop(4);

  for (int gi = 0; gi < kNumGroups; ++gi) {
    const Group& grp = group(gi);
    auto head = in.removeFromTop(22);
    g.setColour(kGroupColour[gi]);
    g.setFont(font(12.0f, true));
    g.drawText(juce::String(grp.name).toUpperCase(), head, juce::Justification::centredLeft);
    for (int s = grp.first; s < grp.first + grp.count; ++s) {
      auto r = in.removeFromTop(19);
      g.setColour(kMuted);
      g.setFont(font(12.0f));
      g.drawText(info(s).name, r.removeFromLeft(92), juce::Justification::centredLeft);
      g.setColour(kFg);
      g.setFont(mono(11.0f));
      g.drawText(formatValue(s, p[(size_t) s]), r.removeFromLeft(78), juce::Justification::centredLeft);
      auto lock = r.removeFromRight(52);
      r.removeFromRight(8);
      const auto track = r.reduced(0, 3);

      const auto bar = track.toFloat().withSizeKeepingCentre((float) track.getWidth(), 5.0f);
      g.setColour(kTile);
      g.fillRoundedRectangle(bar, 2.5f);
      const double v = p[(size_t) s];
      if (view_.zone != Zone::None) {
        const Range z = zoneRange(view_.zone, s);
        if (z.hi - z.lo < 1.0) {
          g.setColour(kGroupColour[gi].withAlpha(0.3f));
          g.fillRect(bar.getX() + bar.getWidth() * (float) z.lo, bar.getBottom() + 2.0f, bar.getWidth() * (float) (z.hi - z.lo), 2.0f);
        }
      }
      g.setColour(kGroupColour[gi]);
      g.fillRoundedRectangle(bar.withWidth(juce::jmax(2.0f, bar.getWidth() * (float) v)), 2.5f);
      hits_.push_back({ track, {}, [this, s](double x) { act([=](Breeder& b) { b.setValue(s, x); }); } });

      if (info(s).bred) {
        const bool locked = view_.locks[(size_t) s];
        button(g, lock, locked ? "Locked" : "Lock", locked, true, [this, s, locked] { act([=](Breeder& b) { b.setLock(s, !locked); }); });
      }
    }
    in.removeFromTop(2);
  }
}

void SynthBreederEditor::mouseDown(const juce::MouseEvent& e) {
  const auto pos = e.getPosition();
  for (auto it = hits_.rbegin(); it != hits_.rend(); ++it) {
    if (!it->area.contains(pos)) continue;
    Hit h = *it;
    if (h.drag) { dragging_ = h; h.drag(juce::jlimit(0.0, 1.0, (pos.x - h.area.getX()) / (double) juce::jmax(1, h.area.getWidth()))); }
    else if (h.click) h.click();
    return;
  }
}

void SynthBreederEditor::mouseDrag(const juce::MouseEvent& e) {
  if (!dragging_) return;
  const auto& a = dragging_->area;
  dragging_->drag(juce::jlimit(0.0, 1.0, (e.getPosition().x - a.getX()) / (double) juce::jmax(1, a.getWidth())));
}

void SynthBreederEditor::mouseUp(const juce::MouseEvent&) { dragging_.reset(); }
