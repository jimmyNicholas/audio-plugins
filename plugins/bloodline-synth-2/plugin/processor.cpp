#include "processor.h"
#include "editor.h"
#include <algorithm>

using namespace bs2;

namespace {
constexpr int kPreviewNote = 48;        // C3, as Bloodline Synth's riff counts it
constexpr double kPreviewSeconds = 0.4; // a clicked tile or pad sounds this long
} // namespace

const char* BloodlineSynth2Processor::riffName(int m) {
  switch (m) {
    case RiffBass: return "Bass";
    case RiffChords: return "Chords";
    case RiffArp: return "Arp";
    case InLoop: return "In loop";
    default: return "Off";
  }
}

BloodlineSynth2Processor::BloodlineSynth2Processor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      studio_(static_cast<std::uint32_t>(juce::Random::getSystemRandom().nextInt())) {
  events_.reserve(2048);
  loopEvents_.reserve(1024);
  riffEvents_.reserve(256);
  midiEngine_.fill(-1);
  padFlash_.fill(100);
  scratchL_.assign(4096, 0.0f);
  scratchR_.assign(4096, 0.0f);
}

void BloodlineSynth2Processor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  const juce::ScopedLock sl(lock_);
  sr_ = sampleRate;
  for (auto& e : engines_) e.prepare(sampleRate);
  riff_.prepare(sampleRate);
  player_.prepare(sampleRate);
  const size_t size = (size_t) juce::jmax(4096, samplesPerBlock);
  scratchL_.assign(size, 0.0f);
  scratchR_.assign(size, 0.0f);
  midiEngine_.fill(-1);
  previewLeft_.fill(0);
}

bool BloodlineSynth2Processor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

const Patch& BloodlineSynth2Processor::patchOf(int engine) const { return patches_[(size_t) engine]; }

int BloodlineSynth2Processor::loopEngine() const {
  if (riffMode_ == InLoop) return kHeard;
  const int s = studio_.loops.playing().sound;
  return s >= 0 && s < Sounds::kPads && studio_.sounds.pad(s).full ? s : -1;
}

void BloodlineSynth2Processor::renderEngines(float* l, float* r, int from, int to) {
  const int n = to - from;
  if (n <= 0) return;
  for (int e = 0; e < kEngines; ++e) {
    auto& engine = engines_[(size_t) e];
    if (engine.sounding() == 0) continue;
    engine.render(patchOf(e), scratchL_.data(), scratchR_.data(), n);
    juce::FloatVectorOperations::add(l + from, scratchL_.data(), n);
    juce::FloatVectorOperations::add(r + from, scratchR_.data(), n);
  }
}

void BloodlineSynth2Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals noDenormals;
  const int n = buffer.getNumSamples();
  buffer.clear();
  double bpm = 120.0, hostBeat = -1.0;
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
  if ((int) scratchL_.size() < n) { scratchL_.assign((size_t) n, 0.0f); scratchR_.assign((size_t) n, 0.0f); }

  // Each engine's patch for this block. A pad emptied while it sounds lets its notes go.
  for (int p = 0; p < Sounds::kPads; ++p) {
    if (studio_.sounds.pad(p).full) patches_[(size_t) p] = studio_.sounds.pad(p).sound;
    else if (engines_[(size_t) p].sounding() > 0) engines_[(size_t) p].allNotesOff();
  }
  patches_[kHeard] = studio_.sounds.heard();

  events_.clear();
  // Clicked tiles and pads: a short C3.
  for (int e = 0; e < kEngines; ++e) {
    if (previewPending_[(size_t) e]) {
      previewPending_[(size_t) e] = false;
      events_.push_back({ 0, e, kPreviewNote, 0.9f, true });
      previewLeft_[(size_t) e] = (int) (kPreviewSeconds * sr_);
    } else if (previewLeft_[(size_t) e] > 0) {
      if (previewLeft_[(size_t) e] <= n) {
        events_.push_back({ previewLeft_[(size_t) e] - 1, e, kPreviewNote, 0.0f, false });
        previewLeft_[(size_t) e] = 0;
      } else previewLeft_[(size_t) e] -= n;
    }
  }

  // Bloodline Synth's riff, on the heard candidate.
  riffEvents_.clear();
  riff_.advance(n, bpm, riffEvents_);
  for (const auto& ev : riffEvents_) events_.push_back({ ev.offset, kHeard, ev.note, ev.velocity, ev.on });

  // MIDI in: the playing loop's sound, or the heard candidate (nothing kept, or In loop).
  for (const auto meta : midi) {
    const auto msg = meta.getMessage();
    const int at = juce::jlimit(0, juce::jmax(0, n - 1), meta.samplePosition);
    if (msg.isNoteOn()) {
      const int le = loopEngine();
      const int engine = riffMode_ == InLoop || le < 0 ? kHeard : le;
      midiEngine_[(size_t) msg.getNoteNumber()] = engine;
      events_.push_back({ at, engine, msg.getNoteNumber(), msg.getFloatVelocity(), true });
    } else if (msg.isNoteOff()) {
      const int engine = midiEngine_[(size_t) msg.getNoteNumber()];
      if (engine >= 0) events_.push_back({ at, engine, msg.getNoteNumber(), 0.0f, false });
      midiEngine_[(size_t) msg.getNoteNumber()] = -1;
    } else if (msg.isAllNotesOff() || msg.isAllSoundOff()) {
      for (auto& e : engines_) e.allNotesOff();
    }
  }

  // The loop.
  loopEvents_.clear();
  player_.advance(studio_.loops.playing(), loopEngine(), n, bpm, hostBeat, play_, loopEvents_);
  for (const auto& ev : loopEvents_)
    if (ev.tag >= 0) events_.push_back({ ev.offset, ev.tag, ev.pitch, ev.velocity, ev.on });

  std::stable_sort(events_.begin(), events_.end(), [](const Event& a, const Event& b) { return a.at < b.at; });

  float* l = buffer.getWritePointer(0);
  float* r = buffer.getWritePointer(1);
  int done = 0;
  for (const auto& ev : events_) {
    if (ev.at > done) { renderEngines(l, r, done, ev.at); done = ev.at; }
    auto& engine = engines_[(size_t) ev.engine];
    if (ev.on) {
      engine.noteOn(ev.pitch, ev.vel, patchOf(ev.engine));
      if (ev.engine == kHeard) tileFlash_ = 0;
      else padFlash_[(size_t) ev.engine] = 0;
    } else engine.noteOff(ev.pitch);
  }
  renderEngines(l, r, done, n);
  midi.clear();
}

juce::AudioProcessorEditor* BloodlineSynth2Processor::createEditor() { return new BloodlineSynth2Editor(*this); }

// The studio's text, then "riff <mode> play <on>".
void BloodlineSynth2Processor::getStateInformation(juce::MemoryBlock& dest) {
  std::string text;
  {
    const juce::ScopedLock sl(lock_);
    text = studio_.save() + "riff " + std::to_string(riffMode_) + " play " + (play_ ? "1" : "0") + "\n";
  }
  dest.replaceAll(text.data(), text.size());
}

void BloodlineSynth2Processor::setStateInformation(const void* data, int size) {
  if (data == nullptr || size <= 0) return;
  const std::string text(static_cast<const char*>(data), (size_t) size);
  const juce::ScopedLock sl(lock_);
  if (!studio_.load(text)) return;
  const auto at = text.rfind("\nriff ");
  if (at != std::string::npos && text.size() >= at + 14) {
    const int mode = text[at + 6] - '0';
    riffMode_ = mode >= 0 && mode < kRiffModes ? mode : RiffOff;
    riff_.setPattern(juce::jlimit(0, 2, riffMode_ - 1));
    riff_.setOn(riffMode_ >= RiffBass && riffMode_ <= RiffArp);
    play_ = text[at + 13] == '1';
  }
}

BloodlineSynth2Processor::View BloodlineSynth2Processor::view() {
  const juce::ScopedLock sl(lock_);
  View v;
  const auto& s = studio_.sounds;
  const auto& sl2 = s.litter();
  for (int i = 0; i < sl2.size(); ++i) v.sounds.push_back(sl2.candidate(i));
  for (int p = 0; p < Sounds::kPads; ++p) {
    v.pads[(size_t) p] = s.pad(p);
    v.padFlash[(size_t) p] = padFlash_[(size_t) p];
    padFlash_[(size_t) p] = std::min(100, padFlash_[(size_t) p] + 1);
  }
  v.tileFlash = tileFlash_;
  tileFlash_ = std::min(100, tileFlash_ + 1);
  v.soundSelected = sl2.selected();
  v.soundParent = sl2.parent();
  v.soundGeneration = sl2.generation();
  v.soundSpread = sl2.spread();
  v.soundBack = sl2.canGoBack();
  v.zone = s.zone();
  for (int k = 0; k < synth::kNumSettings; ++k) v.locks[(size_t) k] = s.locked(k);
  v.riff = riffMode_;

  const auto& lo = studio_.loops;
  const auto& ll = lo.litter();
  for (int i = 0; i < ll.size(); ++i) v.loops.push_back(ll.candidate(i));
  for (int k = 0; k < Loops::kSlots; ++k) v.slots[(size_t) k] = lo.slot(k);
  v.playing = lo.playing();
  v.playingSlot = lo.playingSlot();
  v.loopSelected = ll.selected();
  v.loopParent = ll.parent();
  v.loopGeneration = ll.generation();
  v.loopSpread = ll.spread();
  v.loopBack = ll.canGoBack();
  v.ops = lo.ops();
  v.play = play_;
  v.running = player_.running();
  v.hostPlaying = hostPlaying_;
  v.beat = player_.beat();
  v.bpm = bpm_;
  v.draggedOut = draggedOut_;
  return v;
}

void BloodlineSynth2Processor::edit(const std::function<void(Studio&)>& change) {
  const juce::ScopedLock sl(lock_);
  change(studio_);
}

void BloodlineSynth2Processor::setRiff(int mode) {
  const juce::ScopedLock sl(lock_);
  riffMode_ = mode >= 0 && mode < kRiffModes ? mode : RiffOff;
  if (riffMode_ >= RiffBass && riffMode_ <= RiffArp) riff_.setPattern(riffMode_ - 1);
  riff_.setOn(riffMode_ >= RiffBass && riffMode_ <= RiffArp);
  if (riffMode_ == InLoop) { play_ = true; draggedOut_ = false; }
}

void BloodlineSynth2Processor::setPlay(bool on) {
  const juce::ScopedLock sl(lock_);
  play_ = on;
  if (on) draggedOut_ = false;
}

void BloodlineSynth2Processor::hearSound(int i) {
  const juce::ScopedLock sl(lock_);
  studio_.sounds.select(i);
  if (riffMode_ == RiffOff || (riffMode_ == InLoop && !play_)) previewPending_[kHeard] = true;
}

void BloodlineSynth2Processor::playPad(int p) {
  const juce::ScopedLock sl(lock_);
  if (p >= 0 && p < Sounds::kPads && studio_.sounds.pad(p).full) previewPending_[(size_t) p] = true;
}

juce::File BloodlineSynth2Processor::writeMidiFile() {
  Loop loop;
  juce::String name;
  {
    const juce::ScopedLock sl(lock_);
    loop = studio_.loops.playing();
    const int slot = studio_.loops.playingSlot();
    name = slot >= 0 ? "slot " + juce::String(slot + 1) : juce::String::charToString((juce::juce_wchar) ('A' + studio_.loops.litter().selected()));
    play_ = false; // the clip takes over
    draggedOut_ = true;
  }
  constexpr int kTicks = 960;
  const double end = loop.beats() * kTicks;
  juce::MidiMessageSequence seq;
  for (const Note& n : loop.notes) {
    const double on = n.start * kTicks;
    seq.addEvent(juce::MidiMessage::noteOn(1, n.pitch, juce::jlimit(0.01f, 1.0f, n.velocity)), on);
    seq.addEvent(juce::MidiMessage::noteOff(1, n.pitch), std::min(on + n.length * kTicks, end - 1));
  }
  seq.addEvent(juce::MidiMessage::endOfTrack(), end); // so the clip is the loop's full length
  seq.updateMatchedPairs();
  juce::MidiFile file;
  file.setTicksPerQuarterNote(kTicks);
  file.addTrack(seq);
  auto out = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("Bloodline Synth 2 " + name + ".mid");
  out.deleteFile();
  if (auto stream = out.createOutputStream()) file.writeTo(*stream);
  return out;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BloodlineSynth2Processor(); }
