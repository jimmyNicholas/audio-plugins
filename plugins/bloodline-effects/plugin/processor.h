// The JUCE layer: runs the heard chain on the track's audio (or on the built-in loop), and saves
// the Breeder. A new rack is built on the message thread whenever the heard chain's effects or
// order change, and handed to the Player under the lock; only settings changes go straight in.
#pragma once
#include "breeder.h"
#include "loop.h"
#include "rack.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>

class BloodlineEffectsProcessor : public juce::AudioProcessor {
public:
  BloodlineEffectsProcessor();

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "Bloodline Effects"; }
  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 8.0; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String&) override {}

  void getStateInformation(juce::MemoryBlock& dest) override;
  void setStateInformation(const void* data, int size) override;

  struct View {
    std::vector<fx::Chain> candidates;
    int selected = 0, parent = -1, generation = 0;
    double spread = 0.2, reduction = 1.0;
    bool canGoBack = false, wild = false, bypass = false, loop = false;
    fx::Character character = fx::Character::None;
    double outputDb = 0.0;
  };
  View view() const;

  // For the editor: change the breeder, then the heard chain is put into the Player.
  void edit(const std::function<void(fx::Breeder&)>& change);
  void setBypass(bool on);
  void setLoop(bool on);
  void setOutput(double db);

private:
  void refresh(); // message thread: hand the heard chain to the Player

  fx::Breeder breeder_;
  fx::Player player_;
  fx::Loop loop_;
  bool loopOn_ = false, bypass_ = false;
  double outputDb_ = 0.0;
  double sr_ = 48000.0;
  int maxBlock_ = 512;
  juce::CriticalSection lock_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineEffectsProcessor)
};
