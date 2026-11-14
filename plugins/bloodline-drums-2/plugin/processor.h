// PROTOTYPE (throwaway spike). The JUCE layer: notes 36 to 43 play the keep pads, 44 to 51 pick and
// play litter tiles A to H, the loop plays the heard candidate, and the Kit is saved with the set.
#pragma once
#include "kit.h"
#include "voice.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <functional>

class BloodlineDrums2Processor : public juce::AudioProcessor {
public:
  BloodlineDrums2Processor();

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "Bloodline Drums 2"; }
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
    std::vector<drums2::Values> candidates;
    std::array<drums2::Pad, drums2::Kit::kPads> pads;
    int selected = 0, parent = -1, generation = 0, pattern = 1;
    double spread = 0.25;
    bool canGoBack = false, loopOn = false;
    drums2::Zone zone = drums2::Zone::None;
    std::array<bool, bloodline::kNumSettings> locks{};
    std::array<int, drums2::Kit::kPads> padFlash{}; // frames since each pad last played, for the editor
    int tileFlash = 0;
  };
  View view();

  // For the editor. Each takes the lock briefly; the audio thread holds it while it renders.
  void edit(const std::function<void(drums2::Kit&)>& change);
  void setLoop(bool on, int pattern);
  void hearTile(int i); // selects and plays it
  void playPad(int p);

private:
  drums2::Kit kit_;
  drums2::Loop loop_;
  std::array<bloodline::Voice, drums2::Kit::kPads> padVoices_;
  bloodline::Voice tileVoice_;
  juce::CriticalSection lock_;
  std::vector<drums2::Loop::Hit> loopHits_;
  struct Event { int at, note; float vel; };
  std::vector<Event> events_;
  int pendingTile_ = -1;
  std::array<bool, drums2::Kit::kPads> pendingPad_{};
  std::array<int, drums2::Kit::kPads> padFlash_{};
  int tileFlash_ = 100;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineDrums2Processor)
};
