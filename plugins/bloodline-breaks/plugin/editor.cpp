#include "editor.h"
#include <cmath>

using namespace breaks;

namespace {

const juce::Colour kBg(0xff15171c), kPanel(0xff1f2229), kTile(0xff292d36), kEdge(0xff3a3f4b);
const juce::Colour kFg(0xffe8e6e1), kMuted(0xff8d8f96);
const juce::Colour kAccent(0xffff8c69), kParent(0xffffc857);

// One colour a role.
juce::Colour roleColour(Role r) {
  switch (r) {
    case Role::Kick: return juce::Colour(0xffff6b6b);
    case Role::Snare: return juce::Colour(0xff7cc6fe);
    case Role::Hat: return juce::Colour(0xfff4d35e);
    case Role::Perc: return juce::Colour(0xff9be564);
    case Role::Off:
    default: return juce::Colour(0xff555a66);
  }
}

juce::Font font(float size, bool bold = false) { return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain)); }
juce::Font mono(float size) { return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain)); }

juce::String pct(double v) { return juce::String(juce::roundToInt(v * 100.0)) + "%"; }
juce::String letter(int i) { return juce::String::charToString((juce::juce_wchar) ('A' + i)); }

juce::String pullName(double p) {
  if (p < 0.2) return "loose";
  if (p < 0.45) return "soft";
  if (p < 0.75) return "firm";
  return "tight";
}

constexpr int kWidth = 1080, kHeight = 720;

} // namespace

BloodlineBreaksEditor::BloodlineBreaksEditor(BloodlineBreaksProcessor& p) : AudioProcessorEditor(&p), proc_(p), view_(p.view()) {
  setSize(kWidth, kHeight);
  startTimerHz(30);
}

BloodlineBreaksEditor::~BloodlineBreaksEditor() { stopTimer(); }

void BloodlineBreaksEditor::timerCallback() {
  view_ = proc_.view();
  repaint();
}

void BloodlineBreaksEditor::act(const std::function<void(Breeder&)>& change) {
  proc_.edit(change);
  timerCallback();
}

void BloodlineBreaksEditor::button(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled,
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

void BloodlineBreaksEditor::slider(juce::Graphics& g, juce::Rectangle<int> r, double value, juce::Colour c, std::function<void(double)> drag) {
  const auto bar = r.toFloat().withSizeKeepingCentre((float) r.getWidth(), 6.0f);
  g.setColour(kTile);
  g.fillRoundedRectangle(bar, 3.0f);
  g.setColour(c);
  g.fillRoundedRectangle(bar.withWidth(juce::jmax(3.0f, bar.getWidth() * (float) value)), 3.0f);
  g.fillEllipse(bar.getX() + bar.getWidth() * (float) value - 5.0f, bar.getCentreY() - 5.0f, 10.0f, 10.0f);
  hits_.push_back({ r, {}, std::move(drag) });
}

void BloodlineBreaksEditor::paint(juce::Graphics& g) {
  hits_.clear();
  g.fillAll(kBg);
  g.setColour(kFg);
  g.setFont(font(18.0f, true));
  g.drawText("BLOODLINE BREAKS", 16, 10, 220, 28, juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(mono(11.0f));
  g.drawText("prototype  |  click a bar to hear it loop, Breed to breed from it, Cross to mate it with the one you hear", 210, 10, 700, 28,
             juce::Justification::centredLeft);

  // Style presets: a soft pull, never a rule.
  g.setColour(kMuted);
  g.setFont(font(11.0f, true));
  g.drawText("STYLE", 16, 46, 44, 26, juce::Justification::centredLeft);
  for (int s = 0; s < kNumStyles; ++s)
    button(g, { 60 + s * 68, 46, 68, 26 }, styleInfo((Style) s).name, (int) view_.ops.style == s, true,
           [this, s] { act([s](Breeder& b) { b.setStyle((Style) s); }); });
  button(g, { 540, 46, 44, 26 }, "Back", false, view_.canGoBack, [this] { act([](Breeder& b) { b.back(); }); });
  button(g, { 586, 46, 50, 26 }, "Fresh", false, true, [this] { act([](Breeder& b) { b.fresh(); }); });

  // Play, kit, the swing dial and the pull.
  const juce::String playText = view_.hostPlaying ? "Host" : view_.play ? "Playing" : "Play";
  button(g, { 16, 80, 60, 26 }, playText, view_.play || view_.hostPlaying, true, [this] { proc_.setPlay(!view_.play); timerCallback(); });
  button(g, { 80, 80, 76, 26 }, view_.kit ? "Kit on" : "MIDI only", view_.kit, true, [this] { proc_.setKit(!view_.kit); timerCallback(); });
  g.setColour(kMuted);
  g.setFont(font(11.0f, true));
  g.drawText("SWING", 164, 80, 44, 26, juce::Justification::centredLeft);
  slider(g, { 208, 80, 116, 26 }, (view_.ops.swingDial - kSwingMin) / (kSwingMax - kSwingMin), kAccent,
         [this](double x) { act([x](Breeder& b) { b.setSwingDial(kSwingMin + x * (kSwingMax - kSwingMin)); }); });
  g.setColour(kFg);
  g.setFont(mono(11.0f));
  g.drawText(pct(view_.ops.swingDial), 328, 80, 38, 26, juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(font(11.0f, true));
  g.drawText("PULL", 370, 80, 34, 26, juce::Justification::centredLeft);
  slider(g, { 404, 80, 86, 26 }, view_.ops.pull, kParent, [this](double x) { act([x](Breeder& b) { b.setPull(x); }); });
  g.setColour(kFg);
  g.setFont(mono(11.0f));
  g.drawText(pullName(view_.ops.pull), 494, 80, 44, 26, juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.drawText("gen " + juce::String(view_.generation) + "  " + juce::String(view_.spread, 2), 540, 80, 96, 26, juce::Justification::centredRight);

  for (int i = 0; i < (int) view_.candidates.size(); ++i) {
    const int col = i % 4, row = i / 4;
    paintTile(g, i, { 16 + col * 157, 118 + row * 232, 149, 224 });
  }

  paintTaste(g, { 16, 588, 620, 116 });
  paintInspector(g, { 652, 46, kWidth - 652 - 16, kHeight - 46 - 16 });
}

// Lanes in pad order, 36 at the top.
void BloodlineBreaksEditor::paintGrid(juce::Graphics& g, const Pattern& p, juce::Rectangle<int> area, bool heard, bool big) {
  const auto a = area.toFloat();
  g.setColour(kBg.withAlpha(0.55f));
  g.fillRoundedRectangle(a, 4.0f);
  const float cw = a.getWidth() / kSteps, rh = a.getHeight() / kLanes;
  for (int s = 0; s < kSteps; s += 4) {
    g.setColour(kEdge.withAlpha(0.35f));
    g.fillRect(a.getX() + s * cw, a.getY(), cw, a.getHeight());
  }
  if (heard && view_.running) {
    const float x = a.getX() + (float) (view_.beat / 4.0) * a.getWidth();
    g.setColour(kFg.withAlpha(0.6f));
    g.fillRect(x, a.getY(), 1.5f, a.getHeight());
  }
  for (int l = 0; l < kLanes; ++l) {
    const Role role = view_.ops.roles[(size_t) l];
    const juce::Colour c = roleColour(role);
    for (int s = 0; s < kSteps; ++s) {
      const float v = p.vel[(size_t) l][(size_t) s];
      if (v <= 0.0f) continue;
      // Where it really lands: swing and nudge move the block off its step.
      const double pos = (s % 2 == 0 ? s : (s - 1) + 2.0 * p.swing) + p.nudge[(size_t) l][(size_t) s];
      const float x = a.getX() + (float) pos * cw;
      const float h = (rh - 2.0f) * (0.3f + 0.7f * v);
      const float y = a.getY() + l * rh + (rh - h) / 2.0f;
      const int rolls = juce::jlimit(1, 4, (int) p.roll[(size_t) l][(size_t) s]);
      g.setColour(c.withAlpha(heard ? 1.0f : 0.8f).withMultipliedBrightness(0.6f + 0.4f * v));
      if (rolls == 1) g.fillRoundedRectangle(x + 0.5f, y, juce::jmax(2.0f, cw - 1.0f), h, big ? 2.0f : 1.0f);
      else
        for (int r = 0; r < rolls; ++r)
          g.fillRect(x + 0.5f + r * cw / rolls, y, juce::jmax(1.0f, cw / rolls - 1.0f), h);
    }
  }
}

void BloodlineBreaksEditor::paintTile(juce::Graphics& g, int i, juce::Rectangle<int> area) {
  const Pattern& p = view_.candidates[(size_t) i];
  const bool heard = i == view_.selected, parent = i == view_.parent;
  const auto rect = area.toFloat();
  g.setColour(heard ? kTile.interpolatedWith(kAccent, 0.18f) : kTile);
  g.fillRoundedRectangle(rect, 8.0f);
  g.setColour(heard ? kAccent : kEdge);
  g.drawRoundedRectangle(rect.reduced(heard ? 1.0f : 0.5f), 8.0f, heard ? 2.0f : 1.0f);
  hits_.push_back({ area.withTrimmedBottom(34), [this, i] { act([i](Breeder& b) { b.select(i); }); }, {} });

  auto in = area.reduced(10, 8);
  auto top = in.removeFromTop(22);
  g.setColour(kFg);
  g.setFont(font(18.0f, true));
  g.drawText(letter(i), top, juce::Justification::centredLeft);
  g.setFont(font(12.0f, true));
  if (parent) {
    g.setColour(kParent);
    g.drawText(juce::String::fromUTF8("\xe2\x98\x85 parent"), top, juce::Justification::centredRight);
  } else if (heard) {
    g.setColour(kAccent);
    g.drawText("hearing", top, juce::Justification::centredRight);
  }
  in.removeFromTop(6);
  paintGrid(g, p, in.removeFromTop(110), heard, false);
  in.removeFromTop(6);
  g.setColour(kMuted);
  g.setFont(mono(10.0f));
  g.drawText("swing " + pct(p.swing) + "   " + juce::String(p.hits()) + " hits", in.removeFromTop(16), juce::Justification::centredLeft);

  auto actions = area.reduced(8, 6).removeFromBottom(28);
  const int half = actions.getWidth() / 2;
  button(g, actions.removeFromLeft(half), parent ? "Breed again" : "Breed", false, true,
         [this, i] { act([i](Breeder& b) { b.breedFrom(i); }); });
  button(g, actions, "Cross", false, !heard, [this, i] { act([i](Breeder& b) { b.cross(b.litter().selected(), i); }); });
}

void BloodlineBreaksEditor::paintTaste(juce::Graphics& g, juce::Rectangle<int> area) {
  g.setColour(kPanel);
  g.fillRoundedRectangle(area.toFloat(), 8.0f);
  auto in = area.reduced(12, 8);
  auto head = in.removeFromTop(20);
  g.setColour(kFg);
  g.setFont(font(12.0f, true));
  g.drawText("TASTE", head.removeFromLeft(60), juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  const auto& t = view_.ops.taste;
  g.drawText(t.count == 0 ? juce::String("Nothing learned yet. Every Breed and Cross teaches it what you keep.")
                          : "Learned from " + juce::String(t.count) + " choices; pulls fresh litters " + pct(t.weight()) + " of the way.",
             head.withTrimmedRight(110), juce::Justification::centredLeft);
  button(g, head.removeFromRight(104).withHeight(22), "Forget taste", false, t.count > 0, [this] { act([](Breeder& b) { b.forgetTaste(); }); });
  in.removeFromTop(6);
  auto grid = in.withTrimmedLeft(70).toFloat();
  const float cw = grid.getWidth() / kSteps, rh = grid.getHeight() / kLanes;
  for (int l = 0; l < kLanes; ++l) {
    const Role role = view_.ops.roles[(size_t) l];
    g.setColour(kMuted);
    g.setFont(mono(10.0f));
    g.drawText(juce::String(kFirstNote + l) + " " + roleName(role), in.getX(), (int) (grid.getY() + l * rh), 68, (int) rh,
               juce::Justification::centredLeft);
    for (int s = 0; s < kSteps; ++s) {
      const float o = t.count == 0 ? 0.0f : (float) t.odds[(size_t) l][(size_t) s];
      g.setColour(kTile);
      g.fillRect(grid.getX() + s * cw + 1, grid.getY() + l * rh + 1, cw - 2, rh - 2);
      g.setColour(roleColour(role).withAlpha(o));
      g.fillRect(grid.getX() + s * cw + 1, grid.getY() + l * rh + 1, cw - 2, rh - 2);
    }
  }
}

void BloodlineBreaksEditor::paintInspector(juce::Graphics& g, juce::Rectangle<int> area) {
  g.setColour(kPanel);
  g.fillRoundedRectangle(area.toFloat(), 8.0f);
  auto in = area.reduced(14, 10);
  const Pattern& p = view_.candidates[(size_t) view_.selected];
  g.setColour(kFg);
  g.setFont(font(15.0f, true));
  g.drawText("HEARING " + letter(view_.selected) + (view_.selected == view_.parent ? juce::String::fromUTF8("  \xe2\x98\x85") : juce::String()),
             in.removeFromTop(24), juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawText("Click a step: hit, ghost, rest. Click a role to change it. Locks keep a lane when breeding.", in.removeFromTop(16),
             juce::Justification::centredLeft);
  in.removeFromTop(10);

  // Counts above the grid.
  constexpr int kHead = 118;
  auto counts = in.removeFromTop(16).withTrimmedLeft(kHead);
  const float cw = counts.getWidth() / (float) kSteps;
  const char* names[4] = { "", "e", "&", "a" };
  g.setFont(mono(10.0f));
  for (int s = 0; s < kSteps; ++s) {
    g.setColour(s % 4 == 0 ? kFg : kMuted);
    g.drawText(s % 4 == 0 ? juce::String(s / 4 + 1) : juce::String(names[s % 4]), (int) (counts.getX() + s * cw), counts.getY(), (int) cw, 16,
               juce::Justification::centred);
  }

  auto gridArea = in.removeFromTop(4 * 40);
  for (int l = 0; l < kLanes; ++l) {
    auto row = gridArea.removeFromTop(40);
    auto head = row.removeFromLeft(kHead).reduced(0, 6);
    const Role role = view_.ops.roles[(size_t) l];
    const bool locked = view_.ops.locks[(size_t) l];
    g.setColour(roleColour(role));
    g.setFont(mono(11.0f));
    g.drawText(juce::String(kFirstNote + l), head.removeFromLeft(24), juce::Justification::centredLeft);
    button(g, head.removeFromLeft(50), roleName(role), false, true,
           [this, l, role] { act([=](Breeder& b) { b.setRole(l, (Role) (((int) role + 1) % kNumRoles)); }); });
    head.removeFromLeft(4);
    button(g, head.withWidth(40), locked ? "Held" : "Lock", locked, true, [this, l, locked] { act([=](Breeder& b) { b.setLock(l, !locked); }); });
    for (int s = 0; s < kSteps; ++s) {
      const juce::Rectangle<int> cell((int) (row.getX() + s * cw), row.getY(), (int) cw, row.getHeight());
      hits_.push_back({ cell, [this, l, s] { act([=](Breeder& b) { b.cycleStep(l, s); }); }, {} });
    }
  }
  auto all = juce::Rectangle<int>(in.getX() + kHead, gridArea.getY() - 4 * 40, in.getWidth() - kHead, 4 * 40);
  paintGrid(g, p, all, true, true);

  in.removeFromTop(14);
  auto sw = in.removeFromTop(26);
  g.setColour(kMuted);
  g.setFont(font(11.0f, true));
  g.drawText("THIS BAR'S SWING", sw.removeFromLeft(kHead), juce::Justification::centredLeft);
  g.setColour(kFg);
  g.setFont(mono(11.0f));
  g.drawText(pct(p.swing), sw.removeFromRight(44), juce::Justification::centredRight);
  slider(g, sw.withTrimmedRight(8), (p.swing - kSwingMin) / (kSwingMax - kSwingMin), kAccent,
         [this](double x) { act([x](Breeder& b) { b.setSwing(kSwingMin + x * (kSwingMax - kSwingMin)); }); });

  in.removeFromTop(12);
  const auto& st = styleInfo(view_.ops.style);
  g.setColour(kMuted);
  g.setFont(font(12.0f));
  g.drawFittedText(juce::String(st.name) + " is usually around " + juce::String(st.bpm) + " BPM; Live is at " +
                       juce::String(juce::roundToInt(view_.bpm)) + ". Choosing a style moves the swing dial to " + pct(st.swing) +
                       ". The dial and style pull breeding " + pullName(view_.ops.pull) + "ly; nothing is forced.",
                   in.removeFromTop(48), juce::Justification::topLeft, 3);

  in.removeFromTop(10);
  auto drag = in.removeFromTop(64);
  g.setColour(kTile);
  g.fillRoundedRectangle(drag.toFloat(), 8.0f);
  g.setColour(kAccent);
  g.drawRoundedRectangle(drag.toFloat().reduced(1.0f), 8.0f, 1.5f);
  g.setFont(font(14.0f, true));
  g.drawText("DRAG MIDI  " + letter(view_.selected), drag.getX(), drag.getY() + 12, drag.getWidth(), 20, juce::Justification::centred);
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawText("drag this bar into a clip slot in Live", drag.getX(), drag.getY() + 36, drag.getWidth(), 16, juce::Justification::centred);
  hits_.push_back({ drag, {}, {}, true });

  in.removeFromTop(12);
  g.setColour(kMuted);
  g.setFont(font(11.0f));
  g.drawFittedText("Notes go out on 36 to 39. Kit on plays the built-in sounds too; MIDI only leaves them to the track you route "
                   "this one to (Bloodline Drums, a Drum Rack). Play loops at Live's tempo when Live is stopped; when Live plays, "
                   "the loop follows Live's bar.",
                   in.removeFromTop(64), juce::Justification::topLeft, 5);
}

void BloodlineBreaksEditor::mouseDown(const juce::MouseEvent& e) {
  const auto pos = e.getPosition();
  midiDragStarted_ = false;
  for (auto it = hits_.rbegin(); it != hits_.rend(); ++it) {
    if (!it->area.contains(pos)) continue;
    Hit h = *it;
    if (h.midiOut) dragging_ = h;
    else if (h.drag) { dragging_ = h; h.drag(juce::jlimit(0.0, 1.0, (pos.x - h.area.getX()) / (double) juce::jmax(1, h.area.getWidth()))); }
    else if (h.click) h.click();
    return;
  }
}

void BloodlineBreaksEditor::mouseDrag(const juce::MouseEvent& e) {
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

void BloodlineBreaksEditor::mouseUp(const juce::MouseEvent&) { dragging_.reset(); }
