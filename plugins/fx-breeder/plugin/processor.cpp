#include "processor.h"
#include "editor.h"

using namespace fx;

FxBreederProcessor::FxBreederProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      breeder_(static_cast<std::uint32_t>(juce::Random::getSystemRandom().nextInt())) {
  player_.prepare(sr_, maxBlock_);
  loopOn_ = wrapperType == wrapperType_Standalone; // the Standalone app has no track to listen to
  refresh();
}

void FxBreederProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  Chain heard;
  {
    const juce::ScopedLock sl(lock_);
    sr_ = sampleRate;
    maxBlock_ = juce::jmax(64, samplesPerBlock);
    heard = breeder_.heard();
  }
  auto rack = std::make_unique<Rack>(heard, sampleRate, maxBlock_); // allocates, so outside the lock
  std::array<std::unique_ptr<Rack>, 2> dropped;
  {
    const juce::ScopedLock sl(lock_);
    player_.prepare(sampleRate, maxBlock_);
    dropped = player_.replace(std::move(rack)); // racks built for the old block size must not run
    player_.setBypass(bypass_);
    player_.setOutput(outputDb_);
    loop_.prepare(sampleRate);
  }
}

bool FxBreederProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() &&
         layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void FxBreederProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
  juce::ScopedNoDenormals noDenormals;
  const int n = buffer.getNumSamples();
  if (buffer.getNumChannels() < 2) return;
  double bpm = 90.0;
  if (auto* head = getPlayHead())
    if (auto pos = head->getPosition())
      if (auto b = pos->getBpm()) bpm = *b;

  const juce::ScopedLock sl(lock_);
  float* l = buffer.getWritePointer(0);
  float* r = buffer.getWritePointer(1);
  if (loopOn_) loop_.render(l, r, n, bpm); // the built-in loop replaces the input
  player_.process(l, r, n);
}

juce::AudioProcessorEditor* FxBreederProcessor::createEditor() { return new FxBreederEditor(*this); }

// The breeder's text, then "player <loop> <bypass> <output dB>".
void FxBreederProcessor::getStateInformation(juce::MemoryBlock& dest) {
  std::string text;
  {
    const juce::ScopedLock sl(lock_);
    text = breeder_.save() + "player " + (loopOn_ ? "1 " : "0 ") + (bypass_ ? "1 " : "0 ") + juce::String(outputDb_, 2).toStdString() + "\n";
  }
  dest.replaceAll(text.data(), text.size());
}

void FxBreederProcessor::setStateInformation(const void* data, int size) {
  if (data == nullptr || size <= 0) return;
  const std::string text(static_cast<const char*>(data), (size_t) size);
  {
    const juce::ScopedLock sl(lock_);
    if (!breeder_.load(text)) return;
    const auto at = text.rfind("\nplayer ");
    if (at != std::string::npos) {
      const auto parts = juce::StringArray::fromTokens(juce::String(text.substr(at + 8)), " \n", "");
      if (parts.size() >= 3) {
        loopOn_ = parts[0] == "1";
        bypass_ = parts[1] == "1";
        outputDb_ = juce::jlimit(-24.0, 12.0, parts[2].getDoubleValue());
        player_.setBypass(bypass_);
        player_.setOutput(outputDb_);
      }
    }
  }
  refresh();
}

void FxBreederProcessor::refresh() {
  Chain heard;
  double sr;
  int block;
  {
    const juce::ScopedLock sl(lock_);
    heard = breeder_.heard();
    if (player_.current() && player_.current()->chain().sameShape(heard)) { player_.setSettings(heard); return; }
    sr = sr_;
    block = maxBlock_;
  }
  auto rack = std::make_unique<Rack>(heard, sr, block); // allocates, so outside the lock
  std::unique_ptr<Rack> done;
  {
    const juce::ScopedLock sl(lock_);
    done = player_.use(std::move(rack));
  }
  // `done` is deleted here, on the message thread.
}

FxBreederProcessor::View FxBreederProcessor::view() const {
  const juce::ScopedLock sl(lock_);
  View v;
  const auto& l = breeder_.litter();
  for (int i = 0; i < l.size(); ++i) v.candidates.push_back(l.candidate(i));
  v.selected = l.selected();
  v.parent = l.parent();
  v.generation = l.generation();
  v.spread = l.spread();
  v.canGoBack = l.canGoBack();
  v.wild = breeder_.wild();
  v.character = breeder_.character();
  v.bypass = bypass_;
  v.loop = loopOn_;
  v.outputDb = outputDb_;
  v.reduction = player_.reduction();
  return v;
}

void FxBreederProcessor::edit(const std::function<void(Breeder&)>& change) {
  {
    const juce::ScopedLock sl(lock_);
    change(breeder_);
  }
  refresh();
}

void FxBreederProcessor::setBypass(bool on) {
  const juce::ScopedLock sl(lock_);
  bypass_ = on;
  player_.setBypass(on);
}

void FxBreederProcessor::setLoop(bool on) {
  const juce::ScopedLock sl(lock_);
  loopOn_ = on;
}

void FxBreederProcessor::setOutput(double db) {
  const juce::ScopedLock sl(lock_);
  outputDb_ = juce::jlimit(-24.0, 12.0, db);
  player_.setOutput(outputDb_);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new FxBreederProcessor(); }
