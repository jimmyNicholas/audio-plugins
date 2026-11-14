#include "editor.h"

using namespace bloodline;

namespace {

// From the #38 mock.
const juce::Colour kBg(0xff17181b), kPanel(0xff212226), kPad(0xff2c2d32), kEdge(0xff3a3b41);
const juce::Colour kFg(0xffe8e6e1), kMuted(0xff8d8b86);

juce::Colour zoneColour(Zone z) {
  switch (z) {
    case Zone::Kick: return juce::Colour(0xffe4572e);
    case Zone::Snare: return juce::Colour(0xfff2c14e);
    case Zone::Hats: return juce::Colour(0xff6cc3d5);
    case Zone::None: break;
  }
  return juce::Colour(0xffa7a3b8);
}

// The zone switch's order, as in the mock.
const Zone kZoneOrder[kNumZones] = { Zone::Kick, Zone::Snare, Zone::Hats, Zone::None };

juce::String formatValue(int setting, double norm) {
  const double v = toReal(setting, norm);
  switch (setting) {
    case Pitch: return juce::String(v, v < 100 ? 1 : 0) + " Hz";
    case Sweep: return juce::String(v, 1) + " st";
    case Decay: return juce::String(v * 1000.0, 0) + " ms";
    case BodyNoise: return juce::String(v * 100.0, 0) + "% noise";
    case NoiseTone: return v >= 1000 ? juce::String(v / 1000.0, 1) + " kHz" : juce::String(v, 0) + " Hz";
    default: return juce::String(v, 2);
  }
}

juce::Font font(float size, bool bold = false) {
  return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
}
juce::Font mono(float size) {
  return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain));
}

constexpr int kWidth = 520, kHeight = 720;
constexpr juce::uint32 kFlashMs = 110;

} // namespace

BloodlineEditor::BloodlineEditor(BloodlineProcessor& p) : AudioProcessorEditor(&p), proc_(p), rows_(p.rows()) {
  for (int i = 0; i < kNumPads; ++i) seenHits_[(size_t) i] = proc_.hits(i);
  setSize(kWidth, kHeight);
  startTimerHz(30);
}

BloodlineEditor::~BloodlineEditor() { stopTimer(); }

void BloodlineEditor::timerCallback() {
  rows_ = proc_.rows();
  const auto now = juce::Time::getMillisecondCounter();
  for (int i = 0; i < kNumPads; ++i) {
    const int h = proc_.hits(i);
    if (h != seenHits_[(size_t) i]) { seenHits_[(size_t) i] = h; flashUntil_[(size_t) i] = now + kFlashMs; }
  }
  repaint();
}

void BloodlineEditor::resized() {
  auto area = getLocalBounds().reduced(16);
  area.removeFromTop(30); // title

  // Grid: 36 at bottom left, counting left to right and then up.
  auto grid = area.removeFromTop(4 * 72 + 3 * 8);
  auto tabs = grid.removeFromLeft(24);
  grid.removeFromLeft(8);
  const int padW = (grid.getWidth() - 3 * 8) / 4, padH = 72;
  for (int i = 0; i < kNumPads; ++i) {
    const int col = i % 4, gridRow = 3 - i / 4; // 0 is the top row on screen
    padRects_[(size_t) i] = { grid.getX() + col * (padW + 8), grid.getY() + gridRow * (padH + 8), padW, padH };
  }
  for (int r = 0; r < kNumRows; ++r) // rows index 0 is the top row
    tabRects_[(size_t) r] = { tabs.getX(), tabs.getY() + r * (padH + 8), tabs.getWidth(), padH };

  area.removeFromTop(14);
  inspector_ = area;
  auto in = inspector_.reduced(14);
  in.removeFromTop(26); // heading
  auto zones = in.removeFromTop(26);
  const int zoneW = 70;
  for (int z = 0; z < kNumZones; ++z) zoneRects_[(size_t) z] = { zones.getX() + z * zoneW, zones.getY(), zoneW, 26 };
  in.removeFromTop(10);
  spreadRect_ = in.removeFromTop(20);
  in.removeFromTop(10);
  for (int s = 0; s < kNumSettings; ++s) {
    auto line = in.removeFromTop(26);
    lockRects_[(size_t) s] = line.removeFromRight(54).reduced(0, 3);
    line.removeFromRight(10);
    line.removeFromLeft(100 + 84);
    trackRects_[(size_t) s] = line.withSizeKeepingCentre(line.getWidth(), 12);
  }
}

void BloodlineEditor::paint(juce::Graphics& g) {
  g.fillAll(kBg);
  g.setColour(kFg);
  g.setFont(font(17.0f, true));
  g.drawText("BLOODLINE", 16, 12, 200, 24, juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(mono(11.0f));
  g.drawText("spike  |  notes 36 to 51, MPD226 bank A", 160, 12, kWidth - 176, 24, juce::Justification::centredRight);

  for (int r = 0; r < kNumRows; ++r) {
    const auto rect = tabRects_[(size_t) r].toFloat();
    const auto c = zoneColour(rows_[(size_t) r].zone());
    const bool sel = r == selected_;
    g.setColour(sel ? c : kPad);
    g.fillRoundedRectangle(rect, 4.0f);
    g.setColour(kEdge);
    if (!sel) g.drawRoundedRectangle(rect.reduced(0.5f), 4.0f, 1.0f);
    g.setColour(sel ? kBg : kMuted);
    g.setFont(font(11.0f, sel));
    juce::GlyphArrangement ga; // vertical label, reading upwards
    ga.addLineOfText(g.getCurrentFont(), "ROW " + juce::String(r + 1), 0, 0);
    juce::Path path;
    ga.createPath(path);
    const auto b = path.getBounds();
    path.applyTransform(juce::AffineTransform::translation(-b.getCentreX(), -b.getCentreY())
                            .rotated(-juce::MathConstants<float>::halfPi)
                            .translated(rect.getCentreX(), rect.getCentreY()));
    g.fillPath(path);
  }

  for (int i = 0; i < kNumPads; ++i) paintPad(g, kFirstNote + i);
  paintInspector(g);
}

void BloodlineEditor::paintPad(juce::Graphics& g, int note) {
  const int i = note - kFirstNote;
  const auto rect = padRects_[(size_t) i].toFloat();
  const Pad p = padFor(note);
  const Row* row = p.row >= 0 ? &rows_[(size_t) p.row] : nullptr;
  const auto c = row ? zoneColour(row->zone()) : kMuted;
  const bool flashing = juce::Time::getMillisecondCounter() < flashUntil_[(size_t) i];

  juce::Colour fill = row ? kPad.interpolatedWith(c, 0.10f) : kPad;
  float alpha = 1.0f;
  const Values* bars = nullptr;
  juce::String label;
  if (p.kind == Pad::Candidate) {
    bars = &row->candidate(p.pad);
    label = juce::String::charToString((juce::juce_wchar) ('A' + p.pad));
  } else if (p.kind == Pad::Choose) {
    fill = kPad.interpolatedWith(c, row->lastPressed() >= 0 ? 0.40f : 0.18f);
    label = "Choose";
  } else if (p.kind == Pad::Bottom) {
    label = "Row " + juce::String(p.row + 1);
    if (row->hasChosen()) bars = &row->chosen();
    else alpha = 0.4f;
  } else {
    alpha = 0.35f;
  }
  if (flashing && p.kind != Pad::Empty) fill = kPad.interpolatedWith(c, 0.7f);

  g.setColour(fill.withMultipliedAlpha(alpha));
  g.fillRoundedRectangle(rect, 6.0f);
  g.setColour(kEdge.withMultipliedAlpha(alpha));
  g.drawRoundedRectangle(rect.reduced(0.5f), 6.0f, 1.0f);

  auto in = rect.reduced(8.0f, 6.0f);
  g.setColour(kMuted.withMultipliedAlpha(alpha));
  g.setFont(mono(10.0f));
  g.drawText(juce::String(note), in.removeFromTop(12.0f), juce::Justification::topLeft);
  g.setColour(kFg.withMultipliedAlpha(alpha));
  g.setFont(font(15.0f, true));
  g.drawText(label, in.removeFromTop(20.0f), juce::Justification::centredLeft);

  if (bars) { // a small bar per setting: the sound at a glance
    auto area = in.removeFromBottom(22.0f);
    const float w = area.getWidth() / kNumSettings;
    g.setColour(c.withAlpha(0.7f * alpha));
    for (int s = 0; s < kNumSettings; ++s) {
      const float h = juce::jmax(1.0f, (float) (*bars)[(size_t) s] * area.getHeight());
      g.fillRect(area.getX() + s * w + 1.0f, area.getBottom() - h, w - 2.0f, h);
    }
  }

  if (p.kind == Pad::Candidate) {
    if (row->favourite() == p.pad) {
      g.setColour(c);
      g.drawRoundedRectangle(rect.reduced(1.5f), 6.0f, 2.5f);
      g.setFont(font(14.0f, true));
      g.drawText(juce::String::fromUTF8("\xe2\x98\x85"), rect.reduced(6.0f, 4.0f), juce::Justification::topRight);
    }
    if (row->lastPressed() == p.pad) {
      g.setColour(c);
      g.fillRect(rect.getX() + 8.0f, rect.getBottom() - 4.0f, rect.getWidth() - 16.0f, 2.0f);
    }
  }
}

void BloodlineEditor::paintInspector(juce::Graphics& g) {
  const Row& row = rows_[(size_t) selected_];
  const auto c = zoneColour(row.zone());
  g.setColour(kPanel);
  g.fillRoundedRectangle(inspector_.toFloat(), 8.0f);
  g.setColour(c);
  g.fillRect(inspector_.getX() + 8, inspector_.getY(), inspector_.getWidth() - 16, 3);

  auto in = inspector_.reduced(14);
  g.setFont(font(15.0f, true));
  g.drawText("ROW " + juce::String(selected_ + 1) + "  " + juce::String(zoneName(row.zone())).toUpperCase(),
             in.removeFromTop(26), juce::Justification::centredLeft);

  for (int z = 0; z < kNumZones; ++z) {
    const auto r = zoneRects_[(size_t) z].toFloat().reduced(1.0f);
    const bool on = row.zone() == kZoneOrder[z];
    g.setColour(on ? zoneColour(kZoneOrder[z]) : kPad);
    g.fillRoundedRectangle(r, 4.0f);
    g.setColour(on ? kBg : kMuted);
    g.setFont(font(12.0f, on));
    g.drawText(juce::String(zoneName(kZoneOrder[z])).toUpperCase(), r, juce::Justification::centred);
  }

  // Spread: a meter between its narrowest and widest, its value, and the new-candidate count.
  {
    auto r = spreadRect_;
    g.setColour(kMuted);
    g.setFont(mono(11.0f));
    g.drawText("SPREAD", r.removeFromLeft(60), juce::Justification::centredLeft);
    auto meter = r.removeFromLeft(110).withSizeKeepingCentre(110, 6).toFloat();
    g.setColour(kPad);
    g.fillRoundedRectangle(meter, 3.0f);
    const Tuning& t = row.tuning();
    const float share = (float) ((row.spread() - t.minSpread) / (t.maxSpread - t.minSpread));
    g.setColour(c);
    g.fillRoundedRectangle(meter.withWidth(juce::jmax(3.0f, meter.getWidth() * share)), 3.0f);
    r.removeFromLeft(10);
    g.setColour(kMuted);
    g.drawText(juce::String(row.spread(), 3), r.removeFromLeft(50), juce::Justification::centredLeft);
    g.setColour(c);
    const juce::String dots = juce::String::fromUTF8(row.newInARow() > 0 ? "\xe2\x97\x8f\xe2\x97\x8b" : "\xe2\x97\x8b\xe2\x97\x8b");
    g.drawText(dots, r.removeFromLeft(30), juce::Justification::centredLeft);
    g.setColour(kMuted);
    g.drawText(row.hasChosen() ? "new twice widens" : "press A, B or C, then Choose", r, juce::Justification::centredLeft);
  }

  // One line per setting: the favourite's value (or the last pressed, before a first choice),
  // the spread around it as shading, the zone as a faint band underneath, and a lock.
  const int shown = row.hasChosen() ? row.favourite() : row.lastPressed();
  for (int s = 0; s < kNumSettings; ++s) {
    const auto track = trackRects_[(size_t) s];
    const int y = track.getCentreY() - 13;
    g.setColour(kMuted);
    g.setFont(font(12.0f));
    g.drawText(juce::String(info(s).name).toUpperCase(), inspector_.getX() + 14, y, 100, 26, juce::Justification::centredLeft);
    g.setColour(kFg);
    g.setFont(mono(11.0f));
    if (shown >= 0)
      g.drawText(formatValue(s, row.candidate(shown)[(size_t) s]), inspector_.getX() + 114, y, 84, 26, juce::Justification::centredLeft);

    const auto bar = track.toFloat().withSizeKeepingCentre((float) track.getWidth(), 6.0f);
    g.setColour(kPad);
    g.fillRoundedRectangle(bar, 3.0f);
    if (row.zone() != Zone::None) {
      const Range z = zoneRange(row.zone(), s);
      g.setColour(c.withAlpha(0.35f));
      g.fillRect(bar.getX() + bar.getWidth() * (float) z.lo, bar.getBottom() + 2.0f, bar.getWidth() * (float) (z.hi - z.lo), 2.0f);
    }
    if (shown >= 0) {
      const double v = row.candidate(shown)[(size_t) s];
      const double lo = row.locked(s) ? v : juce::jmax(0.0, v - row.spread());
      const double hi = row.locked(s) ? v : juce::jmin(1.0, v + row.spread());
      g.setColour(c.withAlpha(0.25f));
      g.fillRect(bar.getX() + bar.getWidth() * (float) lo, bar.getY(), bar.getWidth() * (float) (hi - lo), bar.getHeight());
      g.setColour(c);
      g.fillRoundedRectangle(bar.getX() + bar.getWidth() * (float) v - 2.0f, bar.getY() - 3.0f, 4.0f, 12.0f, 1.0f);
    }

    const auto lr = lockRects_[(size_t) s].toFloat();
    const bool locked = row.locked(s);
    g.setColour(locked ? c : kPanel);
    g.fillRoundedRectangle(lr, 4.0f);
    g.setColour(locked ? c : kEdge);
    g.drawRoundedRectangle(lr.reduced(0.5f), 4.0f, 1.0f);
    g.setColour(locked ? kBg : kMuted);
    g.setFont(font(11.0f, locked));
    g.drawText(locked ? "LOCKED" : "LOCK", lr, juce::Justification::centred);
  }
}

void BloodlineEditor::mouseDown(const juce::MouseEvent& e) {
  const auto pos = e.getPosition();
  for (int i = 0; i < kNumPads; ++i)
    if (padRects_[(size_t) i].contains(pos)) { proc_.press(kFirstNote + i, 0.8f); timerCallback(); return; }
  for (int r = 0; r < kNumRows; ++r)
    if (tabRects_[(size_t) r].contains(pos)) { selected_ = r; repaint(); return; }
  const Row& row = rows_[(size_t) selected_];
  for (int z = 0; z < kNumZones; ++z)
    if (zoneRects_[(size_t) z].contains(pos)) {
      // Clicking the zone that is on turns it off (#43); None is always "off".
      const Zone pick = kZoneOrder[z];
      proc_.setZone(selected_, row.zone() == pick ? Zone::None : pick);
      timerCallback();
      return;
    }
  for (int s = 0; s < kNumSettings; ++s)
    if (lockRects_[(size_t) s].contains(pos)) { proc_.setLock(selected_, s, !row.locked(s)); timerCallback(); return; }
}
