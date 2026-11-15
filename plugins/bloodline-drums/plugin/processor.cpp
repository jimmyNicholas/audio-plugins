#include "processor.h"
#include "editor.h"

using namespace bloodline;

BloodlineDrumsProcessor::BloodlineDrumsProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      instrument_(static_cast<std::uint32_t>(juce::Random::getSystemRandom().nextInt())) {}

void BloodlineDrumsProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  const juce::ScopedLock sl(lock_);
  instrument_.prepare(sampleRate);
  left_.assign((size_t) juce::jmax(1, samplesPerBlock), 0.0f);
  right_.assign((size_t) juce::jmax(1, samplesPerBlock), 0.0f);
}

bool BloodlineDrumsProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  const auto out = layouts.getMainOutputChannelSet();
  return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void BloodlineDrumsProcessor::hit(int note) {
  const int pad = note - kFirstNote;
  if (pad >= 0 && pad < kNumPads) hits_[(size_t) pad].fetch_add(1, std::memory_order_relaxed);
}

// Renders in pieces between note-ons, so each hit lands on its own sample.
void BloodlineDrumsProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals noDenormals;
  const int n = buffer.getNumSamples();
  if ((int) left_.size() < n) { left_.resize((size_t) n); right_.resize((size_t) n); } // a bigger block than promised

  const juce::ScopedLock sl(lock_);
  int done = 0;
  for (const auto meta : midi) {
    const auto msg = meta.getMessage();
    if (!msg.isNoteOn()) continue;
    const int at = juce::jlimit(done, n, meta.samplePosition);
    if (at > done) instrument_.render(left_.data() + done, right_.data() + done, at - done);
    done = at;
    instrument_.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
    hit(msg.getNoteNumber());
  }
  if (n > done) instrument_.render(left_.data() + done, right_.data() + done, n - done);

  if (buffer.getNumChannels() >= 2) {
    buffer.copyFrom(0, 0, left_.data(), n);
    buffer.copyFrom(1, 0, right_.data(), n);
    for (int ch = 2; ch < buffer.getNumChannels(); ++ch) buffer.clear(ch, 0, n);
  } else if (buffer.getNumChannels() == 1) {
    buffer.copyFrom(0, 0, left_.data(), n, 0.5f);
    buffer.addFrom(0, 0, right_.data(), n, 0.5f);
  }
}

juce::AudioProcessorEditor* BloodlineDrumsProcessor::createEditor() { return new BloodlineDrumsEditor(*this); }

// The instrument's text, then a line of inspector groups (1 open, 0 closed). Instrument::load reads
// only its own part, so the extra line rides along.
void BloodlineDrumsProcessor::getStateInformation(juce::MemoryBlock& dest) {
  std::string text;
  {
    const juce::ScopedLock sl(lock_);
    text = instrument_.save();
  }
  text += "ui ";
  for (bool open : groupsOpen) text += open ? '1' : '0';
  text += '\n';
  dest.replaceAll(text.data(), text.size());
}

void BloodlineDrumsProcessor::setStateInformation(const void* data, int size) {
  if (data == nullptr || size <= 0) return;
  const std::string text(static_cast<const char*>(data), (size_t) size);
  {
    const juce::ScopedLock sl(lock_);
    if (!instrument_.load(text)) return; // a state from somewhere else, or from the first spike, is ignored
  }
  const auto at = text.rfind("\nui ");
  if (at != std::string::npos && text.size() >= at + 4 + kNumGroups)
    for (int g = 0; g < kNumGroups; ++g) groupsOpen[(size_t) g] = text[at + 4 + (size_t) g] == '1';
}

BloodlineDrumsProcessor::View BloodlineDrumsProcessor::view() const {
  const juce::ScopedLock sl(lock_);
  View v;
  for (int line = 0; line < kNumLines; ++line) {
    if (const Row* r = instrument_.row(line)) v.rows[(size_t) line] = *r;
    v.open[(size_t) line] = instrument_.isOpen(line);
  }
  return v;
}

void BloodlineDrumsProcessor::press(int note, float velocity) {
  {
    const juce::ScopedLock sl(lock_);
    instrument_.noteOn(note, velocity);
  }
  hit(note);
}

void BloodlineDrumsProcessor::edit(const std::function<void(Instrument&)>& change) {
  const juce::ScopedLock sl(lock_);
  change(instrument_);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BloodlineDrumsProcessor(); }
