#include "editor.h"

using namespace bloodline;

namespace {

// juce::String(x, 0) keeps every decimal, so whole numbers are rounded here.
juce::String num(double v, int places) { return places == 0 ? juce::String(juce::roundToInt(v)) : juce::String(v, places); }

// From the #38 mock.
const juce::Colour kBg(0xff17181b), kPanel(0xff212226), kPad(0xff2c2d32), kEdge(0xff3a3b41);
const juce::Colour kFg(0xffe8e6e1), kMuted(0xff8d8b86), kBlank(0xff1d1e21);

juce::Colour zoneColour(Zone z) {
  switch (z) {
    case Zone::Kick: return juce::Colour(0xffe4572e);
    case Zone::Snare: return juce::Colour(0xfff2c14e);
    case Zone::Hats: return juce::Colour(0xff6cc3d5);
    case Zone::None: break;
  }
  return juce::Colour(0xffa7a3b8);
}

const Zone kZoneOrder[kNumZones] = { Zone::Kick, Zone::Snare, Zone::Hats, Zone::None };

juce::String hz(double v) { return v >= 1000 ? num(v / 1000.0, v >= 10000 ? 1 : 2) + " kHz" : num(v, v < 100 ? 1 : 0) + " Hz"; }

juce::String formatValue(int setting, double norm) {
  const double v = toReal(setting, norm);
  const juce::String unit(info(setting).unit);
  if (setting == Pan) return std::abs(v) < 0.01 ? juce::String("C") : juce::String(juce::roundToInt(std::abs(v) * 100.0)) + (v < 0 ? " L" : " R");
  if (setting == Bits) return juce::String(v, 1) + " bits";
  if (unit == "Hz") return hz(v);
  if (unit == "s") return v < 1.0 ? num(v * 1000.0, v < 0.01 ? 1 : 0) + " ms" : juce::String(v, 2) + " s";
  if (unit == "st") return juce::String(v, 1) + " st";
  if (unit == "dB") return juce::String(v, 1) + " dB";
  return juce::String(juce::roundToInt(v * 100.0)) + "%";
}

juce::String lineName(int line) { return line == kBench ? "BENCH" : "SLOT " + juce::String(line); }

juce::Font font(float size, bool bold = false) {
  return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
}
juce::Font mono(float size) {
  return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain));
}

constexpr int kWidth = 1100, kHeight = 720;
constexpr int kLineH = 76, kGap = 8, kPadW = 92;
constexpr juce::uint32 kFlashMs = 110, kConfirmMs = 3000;

} // namespace

BloodlineDrumsEditor::BloodlineDrumsEditor(BloodlineDrumsProcessor& p) : AudioProcessorEditor(&p), proc_(p), view_(p.view()) {
  for (int i = 0; i < kNumPads; ++i) seenHits_[(size_t) i] = proc_.hits(i);
  setSize(kWidth, kHeight);
  startTimerHz(30);
}

BloodlineDrumsEditor::~BloodlineDrumsEditor() { stopTimer(); }

void BloodlineDrumsEditor::timerCallback() {
  view_ = proc_.view();
  const auto now = juce::Time::getMillisecondCounter();
  for (int i = 0; i < kNumPads; ++i) {
    const int h = proc_.hits(i);
    if (h != seenHits_[(size_t) i]) { seenHits_[(size_t) i] = h; flashUntil_[(size_t) i] = now + kFlashMs; }
  }
  repaint();
}

void BloodlineDrumsEditor::act(const std::function<void(Instrument&)>& change) {
  proc_.edit(change);
  timerCallback();
}

void BloodlineDrumsEditor::button(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled,
                             juce::Colour c, std::function<void()> click) {
  const auto f = r.toFloat().reduced(1.0f);
  const float a = enabled ? 1.0f : 0.35f;
  g.setColour((on ? c : kPad).withMultipliedAlpha(a));
  g.fillRoundedRectangle(f, 4.0f);
  g.setColour((on ? c : kEdge).withMultipliedAlpha(a));
  g.drawRoundedRectangle(f.reduced(0.5f), 4.0f, 1.0f);
  g.setColour((on ? kBg : kFg).withMultipliedAlpha(a));
  g.setFont(font(11.0f, on));
  g.drawText(text, r, juce::Justification::centred);
  if (enabled) hits_.push_back({ r, std::move(click), {} });
}

void BloodlineDrumsEditor::slider(juce::Graphics& g, juce::Rectangle<int> track, double value, bool enabled, juce::Colour c,
                             std::function<void(double)> drag) {
  const auto bar = track.toFloat().withSizeKeepingCentre((float) track.getWidth(), 6.0f);
  const float a = enabled ? 1.0f : 0.4f;
  g.setColour(kPad);
  g.fillRoundedRectangle(bar, 3.0f);
  g.setColour(c.withMultipliedAlpha(a));
  g.fillRoundedRectangle(bar.getX() + bar.getWidth() * (float) value - 2.0f, bar.getY() - 4.0f, 4.0f, 14.0f, 1.0f);
  if (enabled) hits_.push_back({ track, {}, std::move(drag) });
}

void BloodlineDrumsEditor::paint(juce::Graphics& g) {
  hits_.clear();
  g.fillAll(kBg);
  g.setColour(kFg);
  g.setFont(font(17.0f, true));
  g.drawText("BLOODLINE DRUMS", 16, 12, 200, 24, juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(mono(11.0f));
  g.drawText("second spike  |  notes 36 to 51, MPD226 bank A", 130, 12, 480, 24, juce::Justification::centredLeft);

  const bool confirming = juce::Time::getMillisecondCounter() < confirmUntil_;
  button(g, { 470, 44, 142, 26 }, confirming ? "Sure? Click again" : "Start again", confirming, true,
         juce::Colour(0xffe4572e), [this, confirming] {
           if (!confirming) { confirmUntil_ = juce::Time::getMillisecondCounter() + kConfirmMs; repaint(); return; }
           confirmUntil_ = 0;
           selected_ = kBench;
           act([](Instrument& i) { i.startAgain(); });
         });

  // Lines from the top: slot 3, 2, 1, then the workbench.
  for (int line = 0; line < kNumLines; ++line) {
    const int y = 84 + (kNumLines - 1 - line) * (kLineH + kGap);
    paintLine(g, line, { 16, y, 600, kLineH });
    if (line == kBench) { // a rule between the kit and the workbench
      g.setColour(kEdge);
      g.fillRect(16, y - kGap / 2 - 1, 596, 1);
    }
  }

  g.setColour(kMuted);
  g.setFont(font(12.0f));
  const juce::String help =
      "Bench: hear A, B or C, then Choose to breed from it. Place (the fourth pad of a blank slot) moves the "
      "bench row into that slot.\n"
      "Placed rows play: A, B, C, and Random (a different pad each hit). Open a placed row to breed it in "
      "place again; Close turns Choose back into Random.\n"
      "Click a line's tab to inspect it. Turn any setting on the last played pad of an open row. Locks keep a "
      "setting as it is when breeding.";
  g.drawFittedText(help, 16, 84 + kNumLines * (kLineH + kGap) + 6, 596, 120, juce::Justification::topLeft, 8);

  paintInspector(g, { 632, 44, kWidth - 632 - 16, kHeight - 44 - 16 });
}

void BloodlineDrumsEditor::paintLine(juce::Graphics& g, int line, juce::Rectangle<int> area) {
  const auto& row = view_.rows[(size_t) line];
  const bool open = view_.open[(size_t) line];
  const auto c = row ? zoneColour(row->zone()) : kMuted;
  const bool sel = line == selected_;

  // The tab: click to inspect.
  auto tab = area.removeFromLeft(64);
  g.setColour(sel ? c : kPad);
  g.fillRoundedRectangle(tab.toFloat(), 4.0f);
  g.setColour(sel ? kBg : kFg);
  g.setFont(font(12.0f, true));
  g.drawText(lineName(line), tab.withTrimmedBottom(kLineH / 2), juce::Justification::centredBottom);
  g.setFont(font(10.0f));
  g.setColour(sel ? kBg : kMuted);
  const juce::String status = !row ? "blank" : line == kBench ? "open" : open ? "open" : "placed";
  g.drawText(status, tab.withTrimmedTop(kLineH / 2 + 2), juce::Justification::centredTop);
  hits_.push_back({ tab, [this, line] { selected_ = line; repaint(); }, {} });

  area.removeFromLeft(8);
  for (int col = 0; col < 4; ++col) {
    paintPad(g, noteFor(line, col), area.removeFromLeft(kPadW));
    area.removeFromLeft(6);
  }
  area.removeFromLeft(4);

  // Actions, two rows of buttons.
  auto top = area.removeFromTop(kLineH / 2).reduced(0, 3), bottom = area.reduced(0, 3);
  const int bw = 44;
  if (line == kBench) {
    button(g, top.removeFromLeft(bw), "Reset", false, true, c, [this] { act([](Instrument& i) { i.reset(kBench); }); });
    return;
  }
  const bool filled = row.has_value();
  button(g, top.removeFromLeft(bw), open ? "Close" : "Open", open, filled, c,
         [this, line, open] { act([line, open](Instrument& i) { i.setOpen(line, !open); }); });
  button(g, top.removeFromLeft(bw), "Reset", false, filled && open, c, [this, line] { act([line](Instrument& i) { i.reset(line); }); });
  button(g, top.removeFromLeft(bw), "Clear", false, filled, c, [this, line] { act([line](Instrument& i) { i.clear(line); }); });
  button(g, bottom.removeFromLeft(bw), juce::String::fromUTF8("\xe2\x96\xb2"), false, filled && line < kNumSlots, c,
         [this, line] { if (selected_ == line) selected_ = line + 1; act([line](Instrument& i) { i.move(line, +1); }); });
  button(g, bottom.removeFromLeft(bw), juce::String::fromUTF8("\xe2\x96\xbc"), false, filled && line > 1, c,
         [this, line] { if (selected_ == line) selected_ = line - 1; act([line](Instrument& i) { i.move(line, -1); }); });
}

void BloodlineDrumsEditor::paintPad(juce::Graphics& g, int note, juce::Rectangle<int> area) {
  const int i = note - kFirstNote;
  const int line = i / 4, col = i % 4;
  const auto& row = view_.rows[(size_t) line];
  const bool open = view_.open[(size_t) line];
  const auto rect = area.toFloat();
  const auto c = row ? zoneColour(row->zone()) : kMuted;
  const bool flashing = juce::Time::getMillisecondCounter() < flashUntil_[(size_t) i];

  juce::Colour fill = row ? kPad.interpolatedWith(c, open ? 0.12f : 0.06f) : kBlank;
  juce::String label;
  const Values* bars = nullptr;
  bool clickable = true;
  if (!row) {
    if (col == 3) { label = "Place"; fill = kPad.interpolatedWith(kFg, 0.08f); }
    else clickable = false;
  } else if (col < 3) {
    bars = &row->candidate(col);
    label = juce::String::charToString((juce::juce_wchar) ('A' + col));
  } else if (open) {
    fill = kPad.interpolatedWith(c, row->lastPressed() >= 0 ? 0.40f : 0.18f);
    label = "Choose";
  } else {
    fill = kPad.interpolatedWith(c, 0.25f);
    label = "Random";
  }
  if (flashing && clickable) fill = kPad.interpolatedWith(c, 0.7f);

  g.setColour(fill);
  g.fillRoundedRectangle(rect, 6.0f);
  g.setColour(row ? kEdge : kEdge.withAlpha(0.5f));
  g.drawRoundedRectangle(rect.reduced(0.5f), 6.0f, 1.0f);

  auto in = rect.reduced(8.0f, 6.0f);
  g.setColour(kMuted);
  g.setFont(mono(10.0f));
  g.drawText(juce::String(note), in.removeFromTop(12.0f), juce::Justification::topLeft);
  g.setColour(kFg);
  g.setFont(font(14.0f, true));
  g.drawText(label, in.removeFromTop(20.0f), juce::Justification::centredLeft);

  if (bars) { // a small bar per setting: the sound at a glance
    auto b = in.removeFromBottom(22.0f);
    const float w = b.getWidth() / kNumSettings;
    g.setColour(c.withAlpha(open ? 0.75f : 0.45f));
    for (int s = 0; s < kNumSettings; ++s) {
      const float h = juce::jmax(1.0f, (float) (*bars)[(size_t) s] * b.getHeight());
      g.fillRect(b.getX() + s * w, b.getBottom() - h, juce::jmax(1.0f, w - 1.0f), h);
    }
  }

  if (row && col < 3) {
    if (row->favourite() == col) {
      g.setColour(c);
      g.drawRoundedRectangle(rect.reduced(1.5f), 6.0f, 2.5f);
      g.setFont(font(14.0f, true));
      g.drawText(juce::String::fromUTF8("\xe2\x98\x85"), rect.reduced(6.0f, 4.0f), juce::Justification::topRight);
    }
    if (row->lastPlayed() == col) {
      g.setColour(c);
      g.fillRect(rect.getX() + 8.0f, rect.getBottom() - 4.0f, rect.getWidth() - 16.0f, 2.0f);
    }
  }
  if (clickable) hits_.push_back({ area, [this, note, line] { selected_ = line; proc_.press(note, 0.8f); timerCallback(); }, {} });
}

void BloodlineDrumsEditor::paintInspector(juce::Graphics& g, juce::Rectangle<int> area) {
  g.setColour(kPanel);
  g.fillRoundedRectangle(area.toFloat(), 8.0f);
  const auto& rowOpt = view_.rows[(size_t) selected_];
  auto in = area.reduced(14, 10);
  if (!rowOpt) {
    g.setColour(kMuted);
    g.setFont(font(13.0f));
    g.drawText(lineName(selected_) + " is blank. Hit its Place pad to move the bench row here.", in.removeFromTop(40),
               juce::Justification::centredLeft);
    return;
  }
  const Row& row = *rowOpt;
  const int line = selected_;
  const bool open = view_.open[(size_t) line];
  const auto c = zoneColour(row.zone());
  g.setColour(c);
  g.fillRect(area.getX() + 8, area.getY(), area.getWidth() - 16, 3);

  g.setColour(kFg);
  g.setFont(font(15.0f, true));
  g.drawText(lineName(line) + "  " + juce::String(zoneName(row.zone())).toUpperCase(), in.removeFromTop(24),
             juce::Justification::centredLeft);
  if (!open) {
    g.setColour(kMuted);
    g.setFont(font(11.0f));
    g.drawText("placed: open it to change zone, locks and settings", in.getX() + 150, in.getY() - 24, in.getWidth() - 150, 24,
               juce::Justification::centredRight);
  }

  auto zones = in.removeFromTop(26);
  for (int z = 0; z < kNumZones; ++z) {
    const Zone pick = kZoneOrder[z];
    // Clicking the zone that is on turns it off (#43); None is always "off".
    button(g, zones.removeFromLeft(70), juce::String(zoneName(pick)).toUpperCase(), row.zone() == pick, open, zoneColour(pick),
           [this, line, pick, current = row.zone()] {
             act([=](Instrument& i) { i.setZone(line, current == pick ? Zone::None : pick); });
           });
  }
  in.removeFromTop(8);

  { // Spread: a meter between its narrowest and widest, its value, and the new-candidate count.
    auto r = in.removeFromTop(18);
    g.setColour(kMuted);
    g.setFont(mono(11.0f));
    g.drawText("SPREAD", r.removeFromLeft(96), juce::Justification::centredLeft);
    auto meter = r.removeFromLeft(110).withSizeKeepingCentre(110, 6).toFloat();
    g.setColour(kPad);
    g.fillRoundedRectangle(meter, 3.0f);
    const Tuning& t = row.tuning();
    const float share = (float) ((row.spread() - t.minSpread) / (t.maxSpread - t.minSpread));
    g.setColour(c);
    g.fillRoundedRectangle(meter.withWidth(juce::jmax(3.0f, meter.getWidth() * share)), 3.0f);
    r.removeFromLeft(10);
    g.setColour(kMuted);
    g.drawText(juce::String(row.spread(), 3), r.removeFromLeft(46), juce::Justification::centredLeft);
    g.setColour(c);
    g.drawText(juce::String::fromUTF8(row.newInARow() > 0 ? "\xe2\x97\x8f\xe2\x97\x8b" : "\xe2\x97\x8b\xe2\x97\x8b"), r,
               juce::Justification::centredLeft);
  }
  in.removeFromTop(6);

  // The row's Pan, Level and Mono: like a mixer channel, so they work placed too.
  auto rowLine = [&](const juce::String& name, const juce::String& value) {
    auto r = in.removeFromTop(22);
    g.setColour(kMuted);
    g.setFont(font(12.0f));
    g.drawText(name, r.removeFromLeft(96), juce::Justification::centredLeft);
    g.setColour(kFg);
    g.setFont(mono(11.0f));
    g.drawText(value, r.removeFromLeft(70), juce::Justification::centredLeft);
    r.removeFromRight(94);
    return r.reduced(0, 4);
  };
  {
    const double pan = row.rowPan();
    auto track = rowLine("ROW PAN", std::abs(pan) < 0.01 ? juce::String("C") : juce::String(juce::roundToInt(std::abs(pan) * 100)) + (pan < 0 ? " L" : " R"));
    slider(g, track, (pan + 1) / 2, true, c, [this, line](double v) { act([=](Instrument& i) { i.setRowPan(line, v * 2 - 1); }); });
    const double db = row.rowLevel();
    auto lt = rowLine("ROW LEVEL", juce::String(db, 1) + " dB");
    slider(g, lt, (db - kRowLevelMin) / (kRowLevelMax - kRowLevelMin), true, c,
           [this, line](double v) { act([=](Instrument& i) { i.setRowLevel(line, kRowLevelMin + v * (kRowLevelMax - kRowLevelMin)); }); });
    button(g, { lt.getRight() + 50, lt.getY() - 3, 44, 20 }, "Mono", row.rowMono(), true, c,
           [this, line, on = row.rowMono()] { act([=](Instrument& i) { i.setRowMono(line, !on); }); });
  }
  in.removeFromTop(6);

  // Which pad the sliders show and turn: the last played.
  const int pad = row.lastPlayed();
  const bool editable = open && pad >= 0;
  {
    auto r = in.removeFromTop(20);
    g.setColour(kFg);
    g.setFont(font(12.0f, true));
    g.drawText(pad >= 0 ? "PAD " + juce::String::charToString((juce::juce_wchar) ('A' + pad)) +
                              (row.favourite() == pad ? juce::String::fromUTF8("  \xe2\x98\x85") : juce::String())
                        : juce::String("PLAY A PAD TO SEE ITS SETTINGS"),
               r, juce::Justification::centredLeft);
  }

  for (int gi = 0; gi < kNumGroups; ++gi) {
    const Group& grp = group(gi);
    const bool groupOpen = proc_.groupsOpen[(size_t) gi];
    auto head = in.removeFromTop(24);
    if (head.getBottom() > area.getBottom()) break;
    g.setColour(kPad);
    g.fillRoundedRectangle(head.toFloat().reduced(0, 2), 3.0f);
    g.setColour(kFg);
    g.setFont(font(12.0f, true));
    g.drawText(juce::String::fromUTF8(groupOpen ? "\xe2\x96\xbe  " : "\xe2\x96\xb8  ") + juce::String(grp.name).toUpperCase(),
               head.reduced(8, 0), juce::Justification::centredLeft);
    hits_.push_back({ head, [this, gi] { proc_.groupsOpen[(size_t) gi] = !proc_.groupsOpen[(size_t) gi]; repaint(); }, {} });
    if (!groupOpen) continue;

    for (int s = grp.first; s < grp.first + grp.count; ++s) {
      auto r = in.removeFromTop(22);
      if (r.getBottom() > area.getBottom()) break;
      g.setColour(kMuted);
      g.setFont(font(12.0f));
      g.drawText(juce::String(info(s).name).toUpperCase(), r.removeFromLeft(96), juce::Justification::centredLeft);
      g.setColour(editable ? kFg : kMuted);
      g.setFont(mono(11.0f));
      if (pad >= 0) g.drawText(formatValue(s, row.candidate(pad)[(size_t) s]), r.removeFromLeft(70), juce::Justification::centredLeft);
      else r.removeFromLeft(70);

      auto lock = r.removeFromRight(46);
      r.removeFromRight(4);
      auto monoR = r.removeFromRight(44);
      r.removeFromRight(6);
      const auto track = r.reduced(0, 4);

      // The zone as a faint band, the spread as shading around the value.
      const auto bar = track.toFloat().withSizeKeepingCentre((float) track.getWidth(), 6.0f);
      if (row.zone() != Zone::None && s <= ClickTone) {
        const Range z = zoneRange(row.zone(), s);
        g.setColour(c.withAlpha(0.35f));
        g.fillRect(bar.getX() + bar.getWidth() * (float) z.lo, bar.getBottom() + 3.0f, bar.getWidth() * (float) (z.hi - z.lo), 2.0f);
      }
      if (pad >= 0) {
        const double v = row.candidate(pad)[(size_t) s];
        if (!row.locked(s) && info(s).bred) {
          const double lo = juce::jmax(0.0, v - row.spread()), hi = juce::jmin(1.0, v + row.spread());
          g.setColour(c.withAlpha(0.18f));
          g.fillRect(bar.getX() + bar.getWidth() * (float) lo, bar.getY(), bar.getWidth() * (float) (hi - lo), bar.getHeight());
        }
        slider(g, track, v, editable, c, [this, line, s](double x) { act([=](Instrument& i) { i.setValue(line, s, x); }); });
      } else {
        g.setColour(kPad);
        g.fillRoundedRectangle(bar, 3.0f);
      }

      if (s == Pan && pad >= 0)
        button(g, monoR, "Mono", row.padMono(pad), editable, c,
               [this, line, pad, on = row.padMono(pad)] { act([=](Instrument& i) { i.setPadMono(line, pad, !on); }); });
      if (info(s).bred)
        button(g, lock, row.locked(s) ? "Locked" : "Lock", row.locked(s), open, c,
               [this, line, s, on = row.locked(s)] { act([=](Instrument& i) { i.setLock(line, s, !on); }); });
    }
  }
}

void BloodlineDrumsEditor::mouseDown(const juce::MouseEvent& e) {
  const auto pos = e.getPosition();
  for (auto it = hits_.rbegin(); it != hits_.rend(); ++it) {
    if (!it->area.contains(pos)) continue;
    Hit h = *it; // painting rebuilds hits_ while this runs
    if (h.drag) {
      dragging_ = h;
      h.drag(juce::jlimit(0.0, 1.0, (pos.x - h.area.getX()) / (double) juce::jmax(1, h.area.getWidth())));
    } else if (h.click) {
      h.click();
    }
    return;
  }
}

void BloodlineDrumsEditor::mouseDrag(const juce::MouseEvent& e) {
  if (!dragging_) return;
  const auto& a = dragging_->area;
  dragging_->drag(juce::jlimit(0.0, 1.0, (e.getPosition().x - a.getX()) / (double) juce::jmax(1, a.getWidth())));
}

void BloodlineDrumsEditor::mouseUp(const juce::MouseEvent&) { dragging_.reset(); }
