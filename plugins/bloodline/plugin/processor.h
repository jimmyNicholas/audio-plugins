// The JUCE layer: forwards notes to the core Instrument, renders it, and saves its state.
// Everything about breeding and sound is in core/.
#pragma once
#include "instrument.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>

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
  double getTailLengthSeconds() const override { return 1.5; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String&) override {}

  void getStateInformation(juce::MemoryBlock& dest) override;
  void setStateInformation(const void* data, int size) override;

  // For the editor. Each takes the lock briefly; the audio thread holds it while it renders.
  void press(int note, float velocity);
  void setZone(int row, bloodline::Zone zone);
  void setLock(int row, int setting, bool on);
  std::array<bloodline::Row, bloodline::kNumRows> rows() const;

  // Counts hits per pad, so the editor can flash pads played from MIDI.
  int hits(int pad) const { return hits_[(size_t) pad].load(std::memory_order_relaxed); }

private:
  void hit(int note);

  bloodline::Instrument instrument_;
  juce::CriticalSection lock_;
  std::vector<float> mono_;
  std::array<std::atomic<int>, bloodline::kNumPads> hits_{};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineProcessor)
};
