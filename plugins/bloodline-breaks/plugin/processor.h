// PROTOTYPE (throwaway spike). The JUCE layer: plays the heard bar as MIDI notes 36 to 39 (and on
// the built-in kit), following the host's bar or free-running, and saves the Breeder.
#pragma once
#include "breeder.h"
#include "player.h"
#include "voice.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <functional>

class BloodlineBreaksProcessor : public juce::AudioProcessor {
public:
  BloodlineBreaksProcessor();

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "Bloodline Breaks"; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return true; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 1.0; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String&) override {}

  void getStateInformation(juce::MemoryBlock& dest) override;
  void setStateInformation(const void* data, int size) override;

  // What the editor draws.
  struct View {
    std::vector<breaks::Pattern> candidates;
    int selected = 0, parent = -1, generation = 0;
    double spread = 0.2, beat = 0.0, bpm = 90.0;
    bool canGoBack = false, play = false, kit = true, running = false, hostPlaying = false;
    breaks::PatternOps ops;
  };
  View view() const;

  // For the editor. Each takes the lock briefly; the audio thread holds it while it renders.
  void edit(const std::function<void(breaks::Breeder&)>& change);
  void setPlay(bool on);
  void setKit(bool on);
  // The heard bar as a one-bar MIDI file, for dragging into Live.
  juce::File writeMidiFile() const;

private:
  breaks::Breeder breeder_;
  breaks::Player player_;
  std::array<bloodline::Voice, breaks::kLanes> voices_;
  bool play_ = false, kit_ = true, hostPlaying_ = false;
  double bpm_ = 90.0;
  juce::CriticalSection lock_;
  std::vector<breaks::NoteEvent> events_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineBreaksProcessor)
};
