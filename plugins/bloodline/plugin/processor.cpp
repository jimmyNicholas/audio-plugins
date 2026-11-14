#include "processor.h"
#include "editor.h"

using namespace bloodline;

BloodlineProcessor::BloodlineProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      instrument_(static_cast<std::uint32_t>(juce::Random::getSystemRandom().nextInt())) {}

void BloodlineProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  const juce::ScopedLock sl(lock_);
  instrument_.prepare(sampleRate);
  mono_.assign((size_t) juce::jmax(1, samplesPerBlock), 0.0f);
}

bool BloodlineProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  const auto out = layouts.getMainOutputChannelSet();
  return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void BloodlineProcessor::hit(int note) {
  const int pad = note - kFirstNote;
  if (pad >= 0 && pad < kNumPads) hits_[(size_t) pad].fetch_add(1, std::memory_order_relaxed);
}

// Renders in pieces between note-ons, so each hit lands on its own sample.
void BloodlineProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals noDenormals;
  const int n = buffer.getNumSamples();
  if ((int) mono_.size() < n) mono_.resize((size_t) n); // a host sent a bigger block than promised

  const juce::ScopedLock sl(lock_);
  int done = 0;
  for (const auto meta : midi) {
    const auto msg = meta.getMessage();
    if (!msg.isNoteOn()) continue;
    const int at = juce::jlimit(done, n, meta.samplePosition);
    if (at > done) instrument_.render(mono_.data() + done, at - done);
    done = at;
    instrument_.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
    hit(msg.getNoteNumber());
  }
  if (n > done) instrument_.render(mono_.data() + done, n - done);

  for (int ch = 0; ch < buffer.getNumChannels(); ++ch) buffer.copyFrom(ch, 0, mono_.data(), n);
}

juce::AudioProcessorEditor* BloodlineProcessor::createEditor() { return new BloodlineEditor(*this); }

void BloodlineProcessor::getStateInformation(juce::MemoryBlock& dest) {
  std::string text;
  {
    const juce::ScopedLock sl(lock_);
    text = instrument_.save();
  }
  dest.replaceAll(text.data(), text.size());
}

void BloodlineProcessor::setStateInformation(const void* data, int size) {
  if (data == nullptr || size <= 0) return;
  const std::string text(static_cast<const char*>(data), (size_t) size);
  const juce::ScopedLock sl(lock_);
  instrument_.load(text); // a state from somewhere else is ignored
}

void BloodlineProcessor::press(int note, float velocity) {
  {
    const juce::ScopedLock sl(lock_);
    instrument_.noteOn(note, velocity);
  }
  hit(note);
}

void BloodlineProcessor::setZone(int row, Zone zone) {
  const juce::ScopedLock sl(lock_);
  instrument_.setZone(row, zone);
}

void BloodlineProcessor::setLock(int row, int setting, bool on) {
  const juce::ScopedLock sl(lock_);
  instrument_.setLock(row, setting, on);
}

std::array<Row, kNumRows> BloodlineProcessor::rows() const {
  const juce::ScopedLock sl(lock_);
  return { instrument_.row(0), instrument_.row(1), instrument_.row(2) };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BloodlineProcessor(); }
