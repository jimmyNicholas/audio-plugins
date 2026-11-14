// The JUCE layer: forwards notes and on-screen actions to the core Instrument, renders it, and saves
// its state. Everything about breeding and sound is in core/.
#pragma once
#include "instrument.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include <functional>
#include <optional>

class BloodlineProcessor : public juce::AudioProcessor {
public:
  BloodlineProcessor();

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "Bloodline"; }
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

  // What the editor draws: each line's row (none for a blank slot) and whether it is open.
  struct View {
    std::array<std::optional<bloodline::Row>, bloodline::kNumLines> rows;
    std::array<bool, bloodline::kNumLines> open{};
  };
  View view() const;

  // For the editor. Each takes the lock briefly; the audio thread holds it while it renders.
  void press(int note, float velocity);
  void edit(const std::function<void(bloodline::Instrument&)>& change);

  // Counts hits per pad, so the editor can flash pads played from MIDI.
  int hits(int pad) const { return hits_[(size_t) pad].load(std::memory_order_relaxed); }

  // Which inspector groups are open; saved with the set (#45). Body is open at first.
  std::array<bool, bloodline::kNumGroups> groupsOpen{ true, false, false, false, false };

private:
  void hit(int note);

  bloodline::Instrument instrument_;
  juce::CriticalSection lock_;
  std::vector<float> left_, right_;
  std::array<std::atomic<int>, bloodline::kNumPads> hits_{};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineProcessor)
};
