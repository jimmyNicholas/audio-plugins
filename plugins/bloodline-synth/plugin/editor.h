// Mouse led: a litter of eight tiles. Click a tile to hear it (under held notes or the riff), Breed
// to breed the litter from it, Cross to mate it with the one you are hearing. A keyboard to play
// with the mouse, the riff to play hands-free, and an inspector with every setting of the heard
// candidate, each with a lock.
#pragma once
#include "processor.h"
#include <functional>
#include <optional>
#include <vector>

class BloodlineSynthEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
  explicit BloodlineSynthEditor(BloodlineSynthProcessor&);
  ~BloodlineSynthEditor() override;

  void paint(juce::Graphics&) override;
  void resized() override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;

private:
  struct Hit {
    juce::Rectangle<int> area;
    std::function<void()> click;
    std::function<void(double)> drag;
  };

  void timerCallback() override;
  void paintTile(juce::Graphics&, int i, juce::Rectangle<int> area);
  void paintInspector(juce::Graphics&, juce::Rectangle<int> area);
  void button(juce::Graphics&, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled,
              std::function<void()> click);
  void act(const std::function<void(synth::Breeder&)>& change);

  BloodlineSynthProcessor& proc_;
  BloodlineSynthProcessor::View view_;
  juce::MidiKeyboardComponent keys_;
  std::vector<Hit> hits_;
  std::optional<Hit> dragging_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineSynthEditor)
};
