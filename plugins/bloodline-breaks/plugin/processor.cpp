#include "processor.h"
#include "editor.h"
#include <algorithm>

using namespace breaks;

BloodlineBreaksProcessor::BloodlineBreaksProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      breeder_(static_cast<std::uint32_t>(juce::Random::getSystemRandom().nextInt())) {
  events_.reserve(kMaxHits * 4);
}

void BloodlineBreaksProcessor::prepareToPlay(double sampleRate, int) {
  const juce::ScopedLock sl(lock_);
  player_.prepare(sampleRate);
  for (auto& v : voices_) v.prepare(sampleRate);
}

bool BloodlineBreaksProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

// The loop's notes go out as MIDI on 36 to 39 and, with the kit on, play the built-in sounds.
// Notes 36 to 39 coming in are passed through and played too, to audition the kit; any other
// MIDI in is dropped.
void BloodlineBreaksProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals noDenormals;
  const int n = buffer.getNumSamples();
  buffer.clear();

  double bpm = 90.0, hostBeat = -1.0;
  bool hostPlaying = false;
  if (auto* head = getPlayHead())
    if (auto pos = head->getPosition()) {
      if (auto b = pos->getBpm()) bpm = *b;
      if (pos->getIsPlaying())
        if (auto ppq = pos->getPpqPosition()) { hostBeat = std::max(0.0, *ppq); hostPlaying = true; }
    }

  const juce::ScopedLock sl(lock_);
  bpm_ = bpm;
  hostPlaying_ = hostPlaying;
  events_.clear();
  for (const auto meta : midi) {
    const auto msg = meta.getMessage();
    const int lane = msg.getNoteNumber() - kFirstNote;
    if (!(msg.isNoteOn() || msg.isNoteOff()) || lane < 0 || lane >= kLanes) continue;
    events_.push_back({ juce::jlimit(0, juce::jmax(0, n - 1), meta.samplePosition), lane, msg.getFloatVelocity(), msg.isNoteOn() });
  }
  player_.advance(breeder_.heard(), n, bpm, hostBeat, play_, events_);
  std::stable_sort(events_.begin(), events_.end(), [](const NoteEvent& a, const NoteEvent& b) { return a.offset < b.offset; });

  midi.clear();
  const auto& roles = breeder_.ops().roles;
  float* l = buffer.getWritePointer(0);
  float* r = buffer.getWritePointer(1);
  int done = 0;
  for (const auto& ev : events_) {
    const int note = kFirstNote + ev.lane;
    if (ev.on) midi.addEvent(juce::MidiMessage::noteOn(1, note, juce::jlimit(0.01f, 1.0f, ev.velocity)), ev.offset);
    else midi.addEvent(juce::MidiMessage::noteOff(1, note), ev.offset);
    if (!ev.on || !kit_ || roles[(size_t) ev.lane] == Role::Off) continue;
    if (ev.offset > done) {
      for (auto& v : voices_) v.render(l + done, r + done, ev.offset - done);
      done = ev.offset;
    }
    voices_[(size_t) ev.lane].trigger(soundFor(roles[(size_t) ev.lane]), ev.velocity);
  }
  if (n > done)
    for (auto& v : voices_) v.render(l + done, r + done, n - done);
}

juce::AudioProcessorEditor* BloodlineBreaksProcessor::createEditor() { return new BloodlineBreaksEditor(*this); }

// The breeder's text, then "play <0|1> kit <0|1>".
void BloodlineBreaksProcessor::getStateInformation(juce::MemoryBlock& dest) {
  std::string text;
  {
    const juce::ScopedLock sl(lock_);
    text = breeder_.save() + "play " + (play_ ? "1" : "0") + " kit " + (kit_ ? "1" : "0") + "\n";
  }
  dest.replaceAll(text.data(), text.size());
}

void BloodlineBreaksProcessor::setStateInformation(const void* data, int size) {
  if (data == nullptr || size <= 0) return;
  const std::string text(static_cast<const char*>(data), (size_t) size);
  const juce::ScopedLock sl(lock_);
  if (!breeder_.load(text)) return;
  const auto at = text.rfind("\nplay ");
  if (at != std::string::npos && text.size() >= at + 13) {
    play_ = text[at + 6] == '1';
    kit_ = text[at + 12] == '1';
  }
}

BloodlineBreaksProcessor::View BloodlineBreaksProcessor::view() const {
  const juce::ScopedLock sl(lock_);
  View v;
  const auto& l = breeder_.litter();
  for (int i = 0; i < l.size(); ++i) v.candidates.push_back(l.candidate(i));
  v.selected = l.selected();
  v.parent = l.parent();
  v.generation = l.generation();
  v.spread = l.spread();
  v.canGoBack = l.canGoBack();
  v.play = play_;
  v.kit = kit_;
  v.running = player_.running();
  v.hostPlaying = hostPlaying_;
  v.beat = player_.beat();
  v.bpm = bpm_;
  v.ops = breeder_.ops();
  return v;
}

void BloodlineBreaksProcessor::edit(const std::function<void(Breeder&)>& change) {
  const juce::ScopedLock sl(lock_);
  change(breeder_);
}

void BloodlineBreaksProcessor::setPlay(bool on) { const juce::ScopedLock sl(lock_); play_ = on; }
void BloodlineBreaksProcessor::setKit(bool on) { const juce::ScopedLock sl(lock_); kit_ = on; }

juce::File BloodlineBreaksProcessor::writeMidiFile() const {
  Pattern p;
  int selected = 0;
  {
    const juce::ScopedLock sl(lock_);
    p = breeder_.heard();
    selected = breeder_.litter().selected();
  }
  constexpr int kTicks = 960;
  juce::MidiMessageSequence seq;
  std::array<Hit, kMaxHits> hits{};
  const int count = hitsOf(p, hits.data(), kMaxHits);
  for (int i = 0; i < count; ++i) {
    const Hit& h = hits[(size_t) i];
    const double on = h.beat * kTicks;
    const int note = kFirstNote + h.lane;
    seq.addEvent(juce::MidiMessage::noteOn(1, note, juce::jlimit(0.01f, 1.0f, h.velocity)), on);
    seq.addEvent(juce::MidiMessage::noteOff(1, note), std::min(on + kTicks / 16.0, 4.0 * kTicks - 1));
  }
  seq.updateMatchedPairs();
  juce::MidiFile file;
  file.setTicksPerQuarterNote(kTicks);
  file.addTrack(seq);
  auto out = juce::File::getSpecialLocation(juce::File::tempDirectory)
                 .getChildFile("Bloodline Breaks " + juce::String::charToString((juce::juce_wchar) ('A' + selected)) + ".mid");
  out.deleteFile();
  if (auto stream = out.createOutputStream()) file.writeTo(*stream);
  return out;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BloodlineBreaksProcessor(); }
