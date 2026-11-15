#include "processor.h"
#include "editor.h"
#include <algorithm>

using namespace drums2;

BloodlineDrums2Processor::BloodlineDrums2Processor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      kit_(static_cast<std::uint32_t>(juce::Random::getSystemRandom().nextInt())) {
  loopHits_.reserve(64);
  events_.reserve(256);
  padFlash_.fill(100);
}

void BloodlineDrums2Processor::prepareToPlay(double sampleRate, int) {
  const juce::ScopedLock sl(lock_);
  for (auto& v : padVoices_) v.prepare(sampleRate);
  tileVoice_.prepare(sampleRate);
  loop_.prepare(sampleRate);
}

bool BloodlineDrums2Processor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void BloodlineDrums2Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals noDenormals;
  const int n = buffer.getNumSamples();
  buffer.clear();
  double bpm = 90.0;
  if (auto* head = getPlayHead())
    if (auto pos = head->getPosition())
      if (auto b = pos->getBpm()) bpm = *b;

  const juce::ScopedLock sl(lock_);
  float* l = buffer.getWritePointer(0);
  float* r = buffer.getWritePointer(1);
  auto renderTo = [&](int& done, int at) {
    if (at <= done) return;
    for (auto& v : padVoices_) v.render(l + done, r + done, at - done);
    tileVoice_.render(l + done, r + done, at - done);
    done = at;
  };

  // The editor's clicks land at the top of the block.
  if (pendingTile_ >= 0) { kit_.select(pendingTile_); tileVoice_.trigger(kit_.heard(), 0.9f); tileFlash_ = 0; pendingTile_ = -1; }
  for (int p = 0; p < Kit::kPads; ++p)
    if (pendingPad_[(size_t) p]) {
      pendingPad_[(size_t) p] = false;
      if (kit_.pad(p).full) { padVoices_[(size_t) p].trigger(kit_.pad(p).sound, 0.9f); padFlash_[(size_t) p] = 0; }
    }

  // MIDI and the loop, merged in time order.
  auto& evs = events_;
  evs.clear();
  for (const auto meta : midi) {
    const auto msg = meta.getMessage();
    if (msg.isNoteOn()) evs.push_back({ juce::jlimit(0, juce::jmax(0, n - 1), meta.samplePosition), msg.getNoteNumber(), msg.getFloatVelocity() });
  }
  loopHits_.clear();
  loop_.advance(n, bpm, loopHits_);
  for (const auto& h : loopHits_) evs.push_back({ h.offset, -1, h.velocity });
  std::stable_sort(evs.begin(), evs.end(), [](const Event& a, const Event& b) { return a.at < b.at; });

  int done = 0;
  for (const auto& e : evs) {
    renderTo(done, e.at);
    if (e.note < 0) { tileVoice_.trigger(kit_.heard(), e.vel); tileFlash_ = 0; continue; }
    const int pad = e.note - Kit::kFirstPad, tile = e.note - Kit::kFirstTile;
    if (pad >= 0 && pad < Kit::kPads && kit_.pad(pad).full) {
      padVoices_[(size_t) pad].trigger(kit_.pad(pad).sound, e.vel);
      padFlash_[(size_t) pad] = 0;
    } else if (tile >= 0 && tile < Kit::kLitter) {
      kit_.select(tile);
      tileVoice_.trigger(kit_.heard(), e.vel);
      tileFlash_ = 0;
    }
  }
  renderTo(done, n);
  midi.clear();
}

juce::AudioProcessorEditor* BloodlineDrums2Processor::createEditor() { return new BloodlineDrums2Editor(*this); }

void BloodlineDrums2Processor::getStateInformation(juce::MemoryBlock& dest) {
  std::string text;
  {
    const juce::ScopedLock sl(lock_);
    text = kit_.save() + "loop " + (loop_.on() ? "1 " : "0 ") + std::to_string(loop_.pattern()) + "\n";
  }
  dest.replaceAll(text.data(), text.size());
}

void BloodlineDrums2Processor::setStateInformation(const void* data, int size) {
  if (data == nullptr || size <= 0) return;
  const std::string text(static_cast<const char*>(data), (size_t) size);
  const juce::ScopedLock sl(lock_);
  if (!kit_.load(text)) return;
  const auto at = text.rfind("\nloop ");
  if (at != std::string::npos && text.size() >= at + 9) {
    loop_.setPattern(text[at + 8] - '0');
    loop_.setOn(text[at + 6] == '1');
  }
}

BloodlineDrums2Processor::View BloodlineDrums2Processor::view() {
  const juce::ScopedLock sl(lock_);
  View v;
  const auto& l = kit_.litter();
  for (int i = 0; i < l.size(); ++i) v.candidates.push_back(l.candidate(i));
  for (int p = 0; p < Kit::kPads; ++p) {
    v.pads[(size_t) p] = kit_.pad(p);
    v.padFlash[(size_t) p] = padFlash_[(size_t) p];
    padFlash_[(size_t) p] = std::min(100, padFlash_[(size_t) p] + 1);
  }
  v.tileFlash = tileFlash_;
  tileFlash_ = std::min(100, tileFlash_ + 1);
  v.selected = l.selected();
  v.parent = l.parent();
  v.generation = l.generation();
  v.spread = l.spread();
  v.canGoBack = l.canGoBack();
  v.loopOn = loop_.on();
  v.pattern = loop_.pattern();
  v.zone = kit_.zone();
  for (int s = 0; s < bloodline::kNumSettings; ++s) v.locks[(size_t) s] = kit_.locked(s);
  return v;
}

void BloodlineDrums2Processor::edit(const std::function<void(Kit&)>& change) {
  const juce::ScopedLock sl(lock_);
  change(kit_);
}

void BloodlineDrums2Processor::setLoop(bool on, int pattern) {
  const juce::ScopedLock sl(lock_);
  loop_.setPattern(pattern);
  loop_.setOn(on);
}

void BloodlineDrums2Processor::hearTile(int i) {
  const juce::ScopedLock sl(lock_);
  kit_.select(i);
  pendingTile_ = i;
}
void BloodlineDrums2Processor::playPad(int p) {
  const juce::ScopedLock sl(lock_);
  if (p >= 0 && p < Kit::kPads) pendingPad_[(size_t) p] = true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BloodlineDrums2Processor(); }
