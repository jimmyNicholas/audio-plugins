#include "processor.h"
#include "editor.h"
#include <algorithm>

using namespace synth;

SynthBreederProcessor::SynthBreederProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      breeder_(static_cast<std::uint32_t>(juce::Random::getSystemRandom().nextInt())) {
  events_.reserve(512);
}

void SynthBreederProcessor::prepareToPlay(double sampleRate, int) {
  const juce::ScopedLock sl(lock_);
  engine_.prepare(sampleRate);
  riff_.prepare(sampleRate);
}

bool SynthBreederProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

// MIDI from the host and the on-screen keyboard, and the riff's notes, in order, each landing on
// its own sample.
void SynthBreederProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals noDenormals;
  const int n = buffer.getNumSamples();
  keyboard.processNextMidiBuffer(midi, 0, n, true);

  double bpm = 96.0;
  if (auto* head = getPlayHead())
    if (auto pos = head->getPosition())
      if (auto b = pos->getBpm()) bpm = *b;

  const juce::ScopedLock sl(lock_);
  events_.clear();
  riff_.advance(n, bpm, events_);
  for (const auto meta : midi) {
    const auto msg = meta.getMessage();
    const int at = juce::jlimit(0, juce::jmax(0, n - 1), meta.samplePosition);
    if (msg.isNoteOn()) events_.push_back({ at, msg.getNoteNumber(), msg.getFloatVelocity(), true });
    else if (msg.isNoteOff()) events_.push_back({ at, msg.getNoteNumber(), 0.0f, false });
    else if (msg.isAllNotesOff() || msg.isAllSoundOff()) engine_.allNotesOff();
  }
  std::stable_sort(events_.begin(), events_.end(), [](const Riff::Event& a, const Riff::Event& b) { return a.offset < b.offset; });

  const Patch patch = breeder_.heard();
  float* l = buffer.getWritePointer(0);
  float* r = buffer.getWritePointer(1);
  int done = 0;
  for (const auto& ev : events_) {
    if (ev.offset > done) { engine_.render(patch, l + done, r + done, ev.offset - done); done = ev.offset; }
    if (ev.on) engine_.noteOn(ev.note, ev.velocity, patch);
    else engine_.noteOff(ev.note);
  }
  if (n > done) engine_.render(patch, l + done, r + done, n - done);
}

juce::AudioProcessorEditor* SynthBreederProcessor::createEditor() { return new SynthBreederEditor(*this); }

// The breeder's text, then "riff <on> <pattern>".
void SynthBreederProcessor::getStateInformation(juce::MemoryBlock& dest) {
  std::string text;
  {
    const juce::ScopedLock sl(lock_);
    text = breeder_.save() + "riff " + (riff_.on() ? "1 " : "0 ") + std::to_string(riff_.pattern()) + "\n";
  }
  dest.replaceAll(text.data(), text.size());
}

void SynthBreederProcessor::setStateInformation(const void* data, int size) {
  if (data == nullptr || size <= 0) return;
  const std::string text(static_cast<const char*>(data), (size_t) size);
  const juce::ScopedLock sl(lock_);
  if (!breeder_.load(text)) return;
  const auto at = text.rfind("\nriff ");
  if (at != std::string::npos && text.size() >= at + 9) {
    riff_.setPattern(text[at + 8] - '0');
    riff_.setOn(text[at + 6] == '1');
  }
}

SynthBreederProcessor::View SynthBreederProcessor::view() const {
  const juce::ScopedLock sl(lock_);
  View v;
  const auto& l = breeder_.litter();
  for (int i = 0; i < l.size(); ++i) v.candidates.push_back(l.candidate(i));
  v.selected = l.selected();
  v.parent = l.parent();
  v.newInARow = l.newInARow();
  v.generation = l.generation();
  v.spread = l.spread();
  v.canGoBack = l.canGoBack();
  v.riffOn = riff_.on();
  v.pattern = riff_.pattern();
  v.zone = breeder_.zone();
  for (int s = 0; s < kNumSettings; ++s) v.locks[(size_t) s] = breeder_.locked(s);
  return v;
}

void SynthBreederProcessor::edit(const std::function<void(Breeder&)>& change) {
  const juce::ScopedLock sl(lock_);
  change(breeder_);
}

void SynthBreederProcessor::setRiff(bool on, int pattern) {
  const juce::ScopedLock sl(lock_);
  riff_.setPattern(pattern);
  riff_.setOn(on);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SynthBreederProcessor(); }
