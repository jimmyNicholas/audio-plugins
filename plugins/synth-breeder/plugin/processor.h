// The JUCE layer: merges MIDI, the on-screen keyboard and the riff into the Engine, playing the
// heard candidate, and saves the Breeder. Everything about breeding and sound is in core/.
#pragma once
#include "breeder.h"
#include "engine.h"
#include "riff.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <functional>

class SynthBreederProcessor : public juce::AudioProcessor {
public:
  SynthBreederProcessor();

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "Synth Breeder"; }
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

  // What the editor draws.
  struct View {
    std::vector<synth::Patch> candidates;
    int selected = 0, parent = -1, newInARow = 0, generation = 0;
    double spread = 0.2;
    bool canGoBack = false, riffOn = false;
    int pattern = 0;
    synth::Zone zone = synth::Zone::None;
    std::array<bool, synth::kNumSettings> locks{};
  };
  View view() const;

  // For the editor. Each takes the lock briefly; the audio thread holds it while it renders.
  void edit(const std::function<void(synth::Breeder&)>& change);
  void setRiff(bool on, int pattern);

  juce::MidiKeyboardState keyboard;

private:
  synth::Breeder breeder_;
  synth::Engine engine_;
  synth::Riff riff_;
  juce::CriticalSection lock_;
  std::vector<synth::Riff::Event> events_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthBreederProcessor)
};
