#include "editor.h"

using namespace fx;

namespace {

const juce::Colour kBg(0xff131518), kPanel(0xff1d2024), kCard(0xff262a30), kEdge(0xff383d45);
const juce::Colour kFg(0xffe8e6e1), kMuted(0xff8b8e94), kAccent(0xff5ee3c1), kParent(0xffffc857), kWarn(0xffff5d5d);

juce::Font font(float size, bool bold = false) { return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain)); }
juce::Font mono(float size) { return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain)); }
juce::Colour typeColour(Type t) { return juce::Colour(info(t).colour); }

constexpr int kWidth = 1100, kHeight = 720;
const Character kCharacters[kNumCharacters] = { Character::None, Character::Space, Character::Dirt, Character::Motion, Character::Glitch };

} // namespace

FxBreederEditor::FxBreederEditor(FxBreederProcessor& p) : AudioProcessorEditor(&p), proc_(p), view_(p.view()) {
  setSize(kWidth, kHeight);
  startTimerHz(30);
}

FxBreederEditor::~FxBreederEditor() { stopTimer(); }

void FxBreederEditor::timerCallback() {
  view_ = proc_.view();
  repaint();
}

void FxBreederEditor::act(const std::function<void(Breeder&)>& change) {
  proc_.edit(change);
  timerCallback();
}

void FxBreederEditor::button(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled,
                             std::function<void()> click, juce::Colour accent) {
  const auto f = r.toFloat().reduced(1.5f);
  const float a = enabled ? 1.0f : 0.35f;
  g.setColour((on ? accent : kCard).withMultipliedAlpha(a));
  g.fillRoundedRectangle(f, 4.0f);
  g.setColour((on ? accent : kEdge).withMultipliedAlpha(a));
  g.drawRoundedRectangle(f.reduced(0.5f), 4.0f, 1.0f);
  g.setColour((on ? kBg : kFg).withMultipliedAlpha(a));
  g.setFont(font(12.0f, on));
  g.drawText(text, r, juce::Justification::centred);
  if (enabled) hits_.push_back({ r, std::move(click), {} });
}

void FxBreederEditor::slider(juce::Graphics& g, juce::Rectangle<int> track, double value, juce::Colour c, std::function<void(double)> drag) {
  const auto bar = track.toFloat().withSizeKeepingCentre((float) track.getWidth(), 5.0f);
  g.setColour(kCard);
  g.fillRoundedRectangle(bar, 2.5f);
  g.setColour(c);
  g.fillRoundedRectangle(bar.withWidth(juce::jmax(2.0f, bar.getWidth() * (float) value)), 2.5f);
  hits_.push_back({ track, {}, std::move(drag) });
}

void FxBreederEditor::paint(juce::Graphics& g) {
  hits_.clear();
  g.fillAll(kBg);
  g.setColour(kFg);
  g.setFont(font(18.0f, true));
  g.drawText("FX BREEDER", 16, 10, 160, 28, juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(mono(11.0f));
  g.drawText("spike  |  on a drum pad's chain or on the drum bus. Click to hear, Breed to breed, Cross to mate", 160, 10, 700, 28,
             juce::Justification::centredLeft);

  // Row 1: character and Wild; litter actions.
  g.setColour(kMuted);
  g.setFont(font(11.0f, true));
  g.drawText("CHARACTER", 16, 46, 80, 26, juce::Justification::centredLeft);
  for (int c = 0; c < kNumCharacters; ++c)
    button(g, { 96 + c * 70, 46, 70, 26 }, characterName(kCharacters[c]), view_.character == kCharacters[c], true,
           [this, c] { act([c](Breeder& b) { b.setCharacter(kCharacters[c]); }); }, kAccent);
  button(g, { 456, 46, 70, 26 }, "Wild", view_.wild, true, [this, w = view_.wild] { act([w](Breeder& b) { b.setWild(!w); }); }, kWarn);
  button(g, { 548, 46, 64, 26 }, "Back", false, view_.canGoBack, [this] { act([](Breeder& b) { b.back(); }); }, kAccent);
  button(g, { 614, 46, 100, 26 }, "Fresh litter", false, true, [this] { act([](Breeder& b) { b.fresh(); }); }, kAccent);

  // Row 1, right: what feeds it, bypass, output, the limiter.
  button(g, { 736, 46, 110, 26 }, view_.loop ? "Loop is input" : "Track is input", view_.loop, true,
         [this] { proc_.setLoop(!view_.loop); timerCallback(); }, kParent);
  button(g, { 848, 46, 72, 26 }, "Bypass", view_.bypass, true, [this] { proc_.setBypass(!view_.bypass); timerCallback(); }, kWarn);
  {
    auto r = juce::Rectangle<int>(932, 46, kWidth - 932 - 16, 26);
    g.setColour(kMuted);
    g.setFont(mono(11.0f));
    g.drawText("OUT " + juce::String(view_.outputDb, 1) + " dB", r.removeFromTop(13), juce::Justification::centredLeft);
    slider(g, r, (view_.outputDb + 24.0) / 36.0, kFg, [this](double v) { proc_.setOutput(-24.0 + v * 36.0); timerCallback(); });
  }

  // Row 2: spread, generation, limiter.
  {
    g.setColour(kMuted);
    g.setFont(mono(11.0f));
    g.drawText("SPREAD", 16, 80, 60, 20, juce::Justification::centredLeft);
    const auto meter = juce::Rectangle<float>(76, 87, 120, 6);
    g.setColour(kCard);
    g.fillRoundedRectangle(meter, 3.0f);
    g.setColour(kAccent);
    g.fillRoundedRectangle(meter.withWidth(juce::jmax(3.0f, meter.getWidth() * (float) ((view_.spread - 0.02) / 0.48))), 3.0f);
    g.setColour(kMuted);
    g.drawText(juce::String(view_.spread, 3) + "   gen " + juce::String(view_.generation) +
                   (view_.wild ? "   wild: effects change, swap, appear and vanish far more often" : ""),
               204, 80, 600, 20, juce::Justification::centredLeft);
    const double cut = -20.0 * std::log10(juce::jmax(1e-4, view_.reduction));
    g.setColour(cut > 0.5 ? kWarn : kMuted);
    g.drawText("LIMITER " + (cut > 0.05 ? "-" + juce::String(cut, 1) + " dB" : juce::String("idle")), 932, 80, 160, 20,
               juce::Justification::centredLeft);
  }

  for (int i = 0; i < (int) view_.candidates.size(); ++i)
    paintCard(g, i, { 16 + (i % 3) * 360, 108 + (i / 3) * 140, 352, 132 });

  paintChain(g, { 16, 392, kWidth - 32, kHeight - 392 - 16 });
}

void FxBreederEditor::paintCard(juce::Graphics& g, int i, juce::Rectangle<int> area) {
  const Chain& c = view_.candidates[(size_t) i];
  const bool heard = i == view_.selected, parent = i == view_.parent;
  const auto rect = area.toFloat();
  g.setColour(heard ? kCard.interpolatedWith(kAccent, 0.15f) : kCard);
  g.fillRoundedRectangle(rect, 8.0f);
  g.setColour(heard ? kAccent : kEdge);
  g.drawRoundedRectangle(rect.reduced(heard ? 1.0f : 0.5f), 8.0f, heard ? 2.0f : 1.0f);
  hits_.push_back({ area.withTrimmedBottom(36), [this, i] { act([i](Breeder& b) { b.select(i); }); }, {} });

  auto in = area.reduced(10, 8);
  auto top = in.removeFromTop(22);
  g.setColour(kFg);
  g.setFont(font(18.0f, true));
  g.drawText(juce::String::charToString((juce::juce_wchar) ('A' + i)), top.removeFromLeft(24), juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(mono(11.0f));
  g.drawText("mix " + juce::String(juce::roundToInt(c.mix * 100)) + "%", top.removeFromLeft(80), juce::Justification::centredLeft);
  if (parent || heard) {
    g.setColour(parent ? kParent : kAccent);
    g.setFont(font(12.0f, true));
    g.drawText(parent ? juce::String::fromUTF8("\xe2\x98\x85 parent") : juce::String("hearing"), top, juce::Justification::centredRight);
  }

  // The chain: a block per effect, in order, with a mini bar for each setting.
  auto chain = in.removeFromTop(58).reduced(0, 4);
  const int w = (chain.getWidth() - (kMaxUnits - 1) * 4) / kMaxUnits;
  for (int k = 0; k < c.count; ++k) {
    const Unit& u = c.units[(size_t) k];
    auto b = juce::Rectangle<int>(chain.getX() + k * (w + 4), chain.getY(), w, chain.getHeight()).toFloat();
    const auto col = typeColour(u.type);
    g.setColour(col.withAlpha(heard ? 0.32f : 0.22f));
    g.fillRoundedRectangle(b, 4.0f);
    if (u.locked) { g.setColour(col); g.drawRoundedRectangle(b.reduced(0.5f), 4.0f, 1.5f); }
    g.setColour(kFg);
    g.setFont(font(10.5f, true));
    g.drawText(info(u.type).name, b.removeFromTop(16), juce::Justification::centred);
    auto bars = b.reduced(5, 4);
    const float bw = bars.getWidth() / kParams;
    g.setColour(col);
    for (int p = 0; p < kParams; ++p) {
      const float h = juce::jmax(1.0f, (float) u.p[(size_t) p] * bars.getHeight());
      g.fillRect(bars.getX() + p * bw + 1, bars.getBottom() - h, bw - 2, h);
    }
    if (k < c.count - 1) {
      g.setColour(kMuted);
      g.fillRect((float) chain.getX() + k * (w + 4) + w, (float) chain.getCentreY(), 4.0f, 1.0f);
    }
  }

  auto actions = area.reduced(8, 6).removeFromBottom(28);
  const int third = actions.getWidth() / 2;
  button(g, actions.removeFromLeft(third), parent ? "Breed again" : "Breed", false, true,
         [this, i] { act([i](Breeder& b) { b.breedFrom(i); }); }, kAccent);
  button(g, actions, "Cross with hearing", false, !heard, [this, i] { act([i](Breeder& b) { b.cross(b.litter().selected(), i); }); }, kAccent);
}

void FxBreederEditor::paintChain(juce::Graphics& g, juce::Rectangle<int> area) {
  g.setColour(kPanel);
  g.fillRoundedRectangle(area.toFloat(), 8.0f);
  const Chain& c = view_.candidates[(size_t) view_.selected];
  auto in = area.reduced(14, 10);
  auto head = in.removeFromTop(24);
  g.setColour(kFg);
  g.setFont(font(15.0f, true));
  g.drawText("HEARING " + juce::String::charToString((juce::juce_wchar) ('A' + view_.selected)), head.removeFromLeft(140),
             juce::Justification::centredLeft);
  g.setColour(kMuted);
  g.setFont(mono(11.0f));
  g.drawText("MIX " + juce::String(juce::roundToInt(c.mix * 100)) + "%", head.removeFromLeft(70), juce::Justification::centredLeft);
  slider(g, head.removeFromLeft(180).reduced(0, 6), c.mix, kAccent, [this](double v) { act([v](Breeder& b) { b.setMix(v); }); });
  g.drawText("   Drag a setting to change it. Lock keeps an effect as it is when breeding. x removes it.", head,
             juce::Justification::centredLeft);
  in.removeFromTop(8);

  const int w = (in.getWidth() - (kMaxUnits - 1) * 10) / kMaxUnits;
  for (int k = 0; k < c.count; ++k) {
    const Unit& u = c.units[(size_t) k];
    const auto col = typeColour(u.type);
    auto col_ = juce::Rectangle<int>(in.getX() + k * (w + 10), in.getY(), w, in.getHeight());
    g.setColour(kCard);
    g.fillRoundedRectangle(col_.toFloat(), 6.0f);
    g.setColour(col);
    g.fillRect(col_.getX() + 6, col_.getY(), col_.getWidth() - 12, 3);

    auto r = col_.reduced(10, 8);
    auto title = r.removeFromTop(24);
    g.setColour(kMuted);
    g.setFont(mono(10.0f));
    g.drawText(juce::String(k + 1), title.removeFromLeft(14), juce::Justification::centredLeft);
    g.setColour(kFg);
    g.setFont(font(14.0f, true));
    g.drawText(info(u.type).name, title, juce::Justification::centredLeft);
    auto controls = r.removeFromTop(26);
    button(g, controls.removeFromLeft(controls.getWidth() - 34), u.locked ? "Locked" : "Lock", u.locked, true,
           [this, k, on = u.locked] { act([=](Breeder& b) { b.setLock(k, !on); }); }, col);
    button(g, controls, "x", false, c.count > 1, [this, k] { act([k](Breeder& b) { b.remove(k); }); }, kWarn);
    r.removeFromTop(10);

    for (int p = 0; p < kParams; ++p) {
      auto line = r.removeFromTop(48);
      char buf[32];
      g.setColour(kMuted);
      g.setFont(font(11.0f));
      auto label = line.removeFromTop(16);
      g.drawText(info(u.type).params[p], label, juce::Justification::centredLeft);
      g.setColour(kFg);
      g.setFont(mono(10.5f));
      g.drawText(formatSetting(u.type, p, u.p[(size_t) p], buf, (int) sizeof(buf)), label, juce::Justification::centredRight);
      slider(g, line.removeFromTop(18), u.p[(size_t) p], col, [this, k, p](double v) { act([=](Breeder& b) { b.setSetting(k, p, v); }); });
    }
  }
  if (c.count < kMaxUnits) {
    auto empty = juce::Rectangle<int>(in.getX() + c.count * (w + 10), in.getY(), in.getRight() - (in.getX() + c.count * (w + 10)), in.getHeight());
    g.setColour(kMuted.withAlpha(0.6f));
    g.setFont(font(12.0f));
    g.drawFittedText("Breeding can add effects here, up to six. Wild adds them more often.", empty.reduced(16), juce::Justification::centred, 3);
  }
}

void FxBreederEditor::mouseDown(const juce::MouseEvent& e) {
  const auto pos = e.getPosition();
  for (auto it = hits_.rbegin(); it != hits_.rend(); ++it) {
    if (!it->area.contains(pos)) continue;
    Hit h = *it;
    if (h.drag) { dragging_ = h; h.drag(juce::jlimit(0.0, 1.0, (pos.x - h.area.getX()) / (double) juce::jmax(1, h.area.getWidth()))); }
    else if (h.click) h.click();
    return;
  }
}

void FxBreederEditor::mouseDrag(const juce::MouseEvent& e) {
  if (!dragging_) return;
  const auto& a = dragging_->area;
  dragging_->drag(juce::jlimit(0.0, 1.0, (e.getPosition().x - a.getX()) / (double) juce::jmax(1, a.getWidth())));
}

void FxBreederEditor::mouseUp(const juce::MouseEvent&) { dragging_.reset(); }
