// Mouse led: six chain cards. Click a card to hear it on the track (or the built-in loop), Breed to
// breed the litter from it, Cross to mate it with the one you are hearing. Below, the heard chain
// laid out effect by effect, each with its four settings, a lock, and a remove button.
#pragma once
#include "processor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <optional>
#include <vector>

class BloodlineEffectsEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
  explicit BloodlineEffectsEditor(BloodlineEffectsProcessor&);
  ~BloodlineEffectsEditor() override;

  void paint(juce::Graphics&) override;
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
  void paintCard(juce::Graphics&, int i, juce::Rectangle<int> area);
  void paintChain(juce::Graphics&, juce::Rectangle<int> area);
  void button(juce::Graphics&, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled,
              std::function<void()> click, juce::Colour accent);
  void slider(juce::Graphics&, juce::Rectangle<int> track, double value, juce::Colour c, std::function<void(double)> drag);
  void act(const std::function<void(fx::Breeder&)>& change);

  BloodlineEffectsProcessor& proc_;
  BloodlineEffectsProcessor::View view_;
  std::vector<Hit> hits_;
  std::optional<Hit> dragging_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineEffectsEditor)
};
