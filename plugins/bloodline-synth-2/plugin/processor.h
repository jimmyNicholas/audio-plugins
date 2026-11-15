// PROTOTYPE (throwaway spike). The JUCE layer: one instrument that breeds synth sounds (left) and
// loops that play them (right), so nothing has to send MIDI to another track. One Bloodline Synth
// engine for each keep pad and one for the heard candidate; a loop's notes go to its sound's engine.
// MIDI in plays the playing loop's sound (the heard candidate when nothing is kept, or In loop).
// The Studio is saved with the set.
#pragma once
#include "engine.h"
#include "riff.h"
#include "studio.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <functional>

class BloodlineSynth2Processor : public juce::AudioProcessor {
public:
  // The left panel's riff row: Off, Bloodline Synth's three riffs, or the right panel's loop.
  enum RiffMode { RiffOff, RiffBass, RiffChords, RiffArp, InLoop, kRiffModes };
  static const char* riffName(int m);

  BloodlineSynth2Processor();

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "Bloodline Synth 2"; }
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
    std::vector<bs2::Patch> sounds;
    std::array<bs2::Pad, bs2::Sounds::kPads> pads;
    int soundSelected = 0, soundParent = -1, soundGeneration = 0, riff = RiffOff;
    double soundSpread = 0.25;
    bool soundBack = false;
    bs2::Zone zone = bs2::Zone::None;
    std::array<bool, synth::kNumSettings> locks{};
    std::array<int, bs2::Sounds::kPads> padFlash{};
    int tileFlash = 100;
    // Right.
    std::vector<bs2::Loop> loops;
    std::array<bs2::Slot, bs2::Loops::kSlots> slots;
    bs2::Loop playing;
    int loopSelected = 0, loopParent = -1, loopGeneration = 0, playingSlot = -1;
    double loopSpread = 0.2, beat = 0.0, bpm = 120.0;
    bool loopBack = false, play = false, running = false, hostPlaying = false, draggedOut = false;
    bs2::LoopOps ops;
  };
  View view();

  // For the editor. Each takes the lock briefly; the audio thread holds it while it renders.
  void edit(const std::function<void(bs2::Studio&)>& change);
  void setRiff(int mode);
  void setPlay(bool on);
  void hearSound(int i); // selects and plays it (a short C3, unless a riff is on)
  void playPad(int p);
  // What is playing, as a MIDI file of the loop's full length, for dragging into Live. Turns
  // Playing off, so the clip takes over.
  juce::File writeMidiFile();

private:
  static constexpr int kHeard = bs2::Sounds::kPads; // the engine index of the heard candidate
  static constexpr int kEngines = bs2::Sounds::kPads + 1;
  const bs2::Patch& patchOf(int engine) const;
  int loopEngine() const; // where the loop's notes go now, or -1 for silence
  void renderEngines(float* l, float* r, int from, int to);

  bs2::Studio studio_;
  synth::Riff riff_;
  bs2::Player player_;
  std::array<synth::Engine, kEngines> engines_;
  std::array<int, 128> midiEngine_{};
  std::vector<float> scratchL_, scratchR_;
  int riffMode_ = RiffOff;
  bool play_ = false, hostPlaying_ = false, draggedOut_ = false;
  double bpm_ = 120.0, sr_ = 48000.0;
  juce::CriticalSection lock_;

  struct Event { int at, engine, pitch; float vel; bool on; };
  std::vector<Event> events_;
  std::vector<bs2::Player::Event> loopEvents_;
  std::vector<synth::Riff::Event> riffEvents_;
  std::array<int, kEngines> previewLeft_{}; // samples until a clicked preview note ends, 0 when none
  std::array<bool, kEngines> previewPending_{};
  int tileFlash_ = 100;
  std::array<int, bs2::Sounds::kPads> padFlash_{};
  std::array<bs2::Patch, kEngines> patches_{}; // each engine's patch; an emptied pad keeps its last

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineSynth2Processor)
};
