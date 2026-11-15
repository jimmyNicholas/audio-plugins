// PROTOTYPE (throwaway spike). The JUCE layer: one instrument that breeds drums (left) and loops
// that play them (right), so nothing has to send MIDI to another track. Notes 36 to 43 play the
// keep pads (always, whatever Playing is) and 48 to 55 switch to a kept loop. The right panel
// sounds only while Playing is on. The Studio is saved with the set.
#pragma once
#include "studio.h"
#include "voice.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <functional>

class BloodlineBreaks2Processor : public juce::AudioProcessor {
public:
  BloodlineBreaks2Processor();

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "Bloodline Breaks 2"; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 4.0; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String&) override {}

  void getStateInformation(juce::MemoryBlock& dest) override;
  void setStateInformation(const void* data, int size) override;

  struct View {
    // Left.
    std::vector<bb2::Values> drums;
    std::array<bb2::Pad, bb2::Drums::kPads> pads;
    int drumSelected = 0, drumParent = -1, drumGeneration = 0, audition = bb2::Audition::InBreak;
    double drumSpread = 0.25;
    bool drumBack = false, auditionOn = false;
    bb2::Zone zone = bb2::Zone::Kick;
    std::array<bool, bloodline::kNumSettings> locks{};
    std::array<int, bb2::Drums::kPads> padFlash{};
    int tileFlash = 100;
    // Right.
    std::vector<bb2::Pattern> loops;
    std::array<bb2::Slot, bb2::Loops::kSlots> slots;
    bb2::Pattern playing;
    int loopSelected = 0, loopParent = -1, loopGeneration = 0, playingSlot = -1;
    double loopSpread = 0.2, beat = 0.0, bpm = 90.0;
    bool loopBack = false, play = false, running = false, hostPlaying = false, anyKept = false;
    bb2::PatternOps ops;
  };
  View view();

  // For the editor. Each takes the lock briefly; the audio thread holds it while it renders.
  void edit(const std::function<void(bb2::Studio&)>& change);
  void setAudition(bool on, int pattern);
  void setPlay(bool on);
  void hearDrum(int i); // selects and plays it
  void playPad(int p);
  // What the loop is playing, as a one-bar MIDI file on 36 to 43, for dragging into Live.
  juce::File writeMidiFile() const;

private:
  bb2::Studio studio_;
  bb2::Audition audition_;
  bb2::Player player_;
  // Two voices a pad, used in turn, so a flam never cuts its own first hit.
  std::array<std::array<bloodline::Voice, 2>, bb2::Drums::kPads> padVoices_;
  std::array<int, bb2::Drums::kPads> nextVoice_{};
  bloodline::Voice tileVoice_;
  bool play_ = false, hostPlaying_ = false;
  double bpm_ = 90.0;
  juce::CriticalSection lock_;

  enum Kind { PadHit, LoopHit, AuditionHit, SlotSwitch };
  struct Event { int at; Kind kind; int index; float vel; int lane = -1; bool primary = true; };
  std::vector<Event> events_;
  std::vector<bb2::Audition::Hit> auditionHits_;
  std::vector<bb2::Player::Event> loopHits_;
  int pendingTile_ = -1;
  std::array<bool, bb2::Drums::kPads> pendingPad_{};
  std::array<int, bb2::Drums::kPads> padFlash_{};
  int tileFlash_ = 100;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineBreaks2Processor)
};
