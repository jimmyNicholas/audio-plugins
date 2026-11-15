#include "processor.h"
#include "editor.h"
#include <algorithm>
#include <cmath>

using namespace bb2;

BloodlineBreaks2Processor::BloodlineBreaks2Processor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      studio_(static_cast<std::uint32_t>(juce::Random::getSystemRandom().nextInt())) {
  events_.reserve(kMaxHits * 4 + 256);
  auditionHits_.reserve(64);
  loopHits_.reserve(kMaxHits * 2);
  padFlash_.fill(100);
}

void BloodlineBreaks2Processor::prepareToPlay(double sampleRate, int) {
  const juce::ScopedLock sl(lock_);
  for (auto& pair : padVoices_)
    for (auto& v : pair) v.prepare(sampleRate);
  tileVoice_.prepare(sampleRate);
  audition_.prepare(sampleRate);
  player_.prepare(sampleRate);
}

bool BloodlineBreaks2Processor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void BloodlineBreaks2Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
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
  float* l = buffer.getWritePointer(0);
  float* r = buffer.getWritePointer(1);
  int done = 0;
  auto renderTo = [&](int at) {
    if (at <= done) return;
    for (auto& pair : padVoices_)
      for (auto& v : pair) v.render(l + done, r + done, at - done);
    tileVoice_.render(l + done, r + done, at - done);
    done = at;
  };

  // The editor's clicks land at the top of the block.
  if (pendingTile_ >= 0) {
    studio_.drums.select(pendingTile_);
    tileVoice_.trigger(studio_.drums.heard(), 0.9f);
    tileFlash_ = 0;
    pendingTile_ = -1;
  }
  for (int p = 0; p < Drums::kPads; ++p)
    if (pendingPad_[(size_t) p]) {
      pendingPad_[(size_t) p] = false;
      if (studio_.drums.pad(p).full) {
        padVoices_[(size_t) p][(size_t) nextVoice_[(size_t) p]].trigger(studio_.drums.pad(p).sound, 0.9f);
        nextVoice_[(size_t) p] ^= 1;
        padFlash_[(size_t) p] = 0;
      }
    }

  // MIDI in, the audition rhythm and the loop, merged in time order.
  events_.clear();
  for (const auto meta : midi) {
    const auto msg = meta.getMessage();
    if (!msg.isNoteOn()) continue;
    const int at = juce::jlimit(0, juce::jmax(0, n - 1), meta.samplePosition), note = msg.getNoteNumber();
    const float vel = msg.getFloatVelocity();
    if (note >= kFirstPadNote && note < kFirstPadNote + Drums::kPads) events_.push_back({ at, PadHit, note - kFirstPadNote, vel });
    else if (note >= kFirstSlotNote && note < kFirstSlotNote + Loops::kSlots) events_.push_back({ at, SlotSwitch, note - kFirstSlotNote, vel });
  }
  auditionHits_.clear();
  audition_.advance(n, bpm, auditionHits_);
  for (const auto& h : auditionHits_) events_.push_back({ h.offset, AuditionHit, 0, h.velocity });
  loopHits_.clear();
  player_.advance(studio_.loops.playing(), studio_.loops.ops(), n, bpm, hostBeat, play_, loopHits_);
  for (const auto& h : loopHits_) events_.push_back({ h.offset, LoopHit, h.pad, h.velocity, h.lane, h.primary });
  std::stable_sort(events_.begin(), events_.end(), [](const Event& a, const Event& b) { return a.at < b.at; });

  const int standIn = audition_.inBreak() ? laneFor(studio_.drums.zone()) : -1;
  auto playPadNow = [&](int p, float vel) {
    if (!studio_.drums.pad(p).full) return false;
    padVoices_[(size_t) p][(size_t) nextVoice_[(size_t) p]].trigger(studio_.drums.pad(p).sound, vel);
    nextVoice_[(size_t) p] ^= 1;
    padFlash_[(size_t) p] = 0;
    return true;
  };
  for (const auto& e : events_) {
    renderTo(e.at);
    switch (e.kind) {
      case PadHit: playPadNow(e.index, e.vel); break;
      case SlotSwitch: studio_.loops.playSlot(e.index); break;
      case AuditionHit: tileVoice_.trigger(studio_.drums.heard(), e.vel); tileFlash_ = 0; break;
      case LoopHit:
        if (e.lane == standIn) {
          if (e.primary) { tileVoice_.trigger(studio_.drums.heard(), e.vel); tileFlash_ = 0; }
          break;
        }
        if (e.index >= 0) playPadNow(e.index, e.vel);
        break;
    }
  }
  renderTo(n);
  midi.clear();
}

juce::AudioProcessorEditor* BloodlineBreaks2Processor::createEditor() { return new BloodlineBreaks2Editor(*this); }

// The studio's text, then "audition <on> <pattern> play <on>".
void BloodlineBreaks2Processor::getStateInformation(juce::MemoryBlock& dest) {
  std::string text;
  {
    const juce::ScopedLock sl(lock_);
    text = studio_.save() + "audition " + (audition_.on() ? "1 " : "0 ") + std::to_string(audition_.pattern()) + " play " +
           (play_ ? "1" : "0") + "\n";
  }
  dest.replaceAll(text.data(), text.size());
}

void BloodlineBreaks2Processor::setStateInformation(const void* data, int size) {
  if (data == nullptr || size <= 0) return;
  const std::string text(static_cast<const char*>(data), (size_t) size);
  const juce::ScopedLock sl(lock_);
  if (!studio_.load(text)) return;
  const auto at = text.rfind("\naudition ");
  if (at != std::string::npos && text.size() >= at + 20) {
    audition_.setPattern(text[at + 12] - '0');
    audition_.setOn(text[at + 10] == '1');
    play_ = text[at + 19] == '1';
  }
}

BloodlineBreaks2Processor::View BloodlineBreaks2Processor::view() {
  const juce::ScopedLock sl(lock_);
  View v;
  const auto& d = studio_.drums;
  const auto& dl = d.litter();
  for (int i = 0; i < dl.size(); ++i) v.drums.push_back(dl.candidate(i));
  for (int p = 0; p < Drums::kPads; ++p) {
    v.pads[(size_t) p] = d.pad(p);
    v.padFlash[(size_t) p] = padFlash_[(size_t) p];
    padFlash_[(size_t) p] = std::min(100, padFlash_[(size_t) p] + 1);
  }
  v.tileFlash = tileFlash_;
  tileFlash_ = std::min(100, tileFlash_ + 1);
  v.drumSelected = dl.selected();
  v.drumParent = dl.parent();
  v.drumGeneration = dl.generation();
  v.drumSpread = dl.spread();
  v.drumBack = dl.canGoBack();
  v.zone = d.zone();
  for (int s = 0; s < bloodline::kNumSettings; ++s) v.locks[(size_t) s] = d.locked(s);
  v.auditionOn = audition_.on();
  v.audition = audition_.pattern();

  const auto& lo = studio_.loops;
  const auto& ll = lo.litter();
  for (int i = 0; i < ll.size(); ++i) v.loops.push_back(ll.candidate(i));
  for (int s = 0; s < Loops::kSlots; ++s) v.slots[(size_t) s] = lo.slot(s);
  v.playing = lo.playing();
  v.playingSlot = lo.playingSlot();
  v.loopSelected = ll.selected();
  v.loopParent = ll.parent();
  v.loopGeneration = ll.generation();
  v.loopSpread = ll.spread();
  v.loopBack = ll.canGoBack();
  v.ops = lo.ops();
  v.anyKept = lo.ops().anyKept();
  v.play = play_;
  v.running = player_.running();
  v.hostPlaying = hostPlaying_;
  v.beat = player_.beat();
  v.bpm = bpm_;
  return v;
}

void BloodlineBreaks2Processor::edit(const std::function<void(Studio&)>& change) {
  const juce::ScopedLock sl(lock_);
  change(studio_);
}

// Choosing In break turns Playing on, so the candidate is heard in the beat.
void BloodlineBreaks2Processor::setAudition(bool on, int pattern) {
  const juce::ScopedLock sl(lock_);
  audition_.setPattern(pattern);
  audition_.setOn(on);
  if (audition_.inBreak()) play_ = true;
}

void BloodlineBreaks2Processor::setPlay(bool on) { const juce::ScopedLock sl(lock_); play_ = on; }

void BloodlineBreaks2Processor::hearDrum(int i) {
  const juce::ScopedLock sl(lock_);
  studio_.drums.select(i);
  pendingTile_ = i;
}

void BloodlineBreaks2Processor::playPad(int p) {
  const juce::ScopedLock sl(lock_);
  if (p >= 0 && p < Drums::kPads) pendingPad_[(size_t) p] = true;
}

// Each hit goes on the notes of the pads its lane plays (a stack on both, a flam's second note at
// its delay and level), so the clip plays back through this plugin as it sounded. Lanes with no
// kept pad are left out.
juce::File BloodlineBreaks2Processor::writeMidiFile() const {
  Pattern p;
  PatternOps ops;
  double bpm = 90.0;
  juce::String name;
  {
    const juce::ScopedLock sl(lock_);
    p = studio_.loops.playing();
    ops = studio_.loops.ops();
    bpm = bpm_;
    const int slot = studio_.loops.playingSlot();
    name = slot >= 0 ? "slot " + juce::String(slot + 1) : juce::String::charToString((juce::juce_wchar) ('A' + studio_.loops.litter().selected()));
  }
  constexpr int kTicks = 960;
  juce::MidiMessageSequence seq;
  std::vector<Hit> hits((size_t) kMaxHits);
  const int count = hitsOf(p, ops, hits.data(), kMaxHits);
  auto add = [&](int note, double on, float vel) {
    seq.addEvent(juce::MidiMessage::noteOn(1, note, juce::jlimit(0.01f, 1.0f, vel)), on);
    seq.addEvent(juce::MidiMessage::noteOff(1, note), std::min(on + kTicks / 16.0, 4.0 * kTicks - 1));
  };
  for (int i = 0; i < count; ++i) {
    const Hit& h = hits[(size_t) i];
    if (h.pad < 0) continue;
    const double beat = std::fmod(h.beat + h.delayMs * 0.001 * juce::jlimit(20.0, 400.0, bpm) / 60.0, 4.0);
    add(kFirstPadNote + h.pad, beat * kTicks, h.velocity);
  }
  seq.updateMatchedPairs();
  juce::MidiFile file;
  file.setTicksPerQuarterNote(kTicks);
  file.addTrack(seq);
  auto out = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("Bloodline Breaks 2 " + name + ".mid");
  out.deleteFile();
  if (auto stream = out.createOutputStream()) file.writeTo(*stream);
  return out;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BloodlineBreaks2Processor(); }
