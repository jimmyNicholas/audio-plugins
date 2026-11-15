// PROTOTYPE (throwaway spike). Laid out like Bloodline Synth: the litter of eight above (click to
// hear, Breed, Cross), the inspector on the right, and eight keep pads where the synth's keyboard
// is. Drag a tile onto a pad to keep it; drag a pad onto another to swap them, or back onto the
// litter to breed from it.
#pragma once
#include "processor.h"
#include <functional>
#include <optional>
#include <vector>

class BloodlineDrums2Editor : public juce::AudioProcessorEditor, private juce::Timer {
public:
  explicit BloodlineDrums2Editor(BloodlineDrums2Processor&);
  ~BloodlineDrums2Editor() override;

  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;

private:
  struct Hit {
    juce::Rectangle<int> area;
    std::function<void()> click;
    std::function<void(double)> drag;
    int tile = -1, pad = -1; // something that can be carried
  };
  struct Carry { int tile = -1, pad = -1; juce::Point<int> start, at; bool moving = false; };

  void timerCallback() override;
  void paintTile(juce::Graphics&, int i, juce::Rectangle<int> area);
  void paintPads(juce::Graphics&, juce::Rectangle<int> area);
  void paintInspector(juce::Graphics&, juce::Rectangle<int> area);
  void paintSound(juce::Graphics&, const drums2::Values&, juce::Rectangle<int> area, float alpha);
  void button(juce::Graphics&, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled,
              std::function<void()> click);
  void act(const std::function<void(drums2::Kit&)>& change);
  int padAt(juce::Point<int>) const;

  BloodlineDrums2Processor& proc_;
  BloodlineDrums2Processor::View view_;
  std::vector<Hit> hits_;
  std::array<juce::Rectangle<int>, drums2::Kit::kPads> padAreas_;
  juce::Rectangle<int> litterArea_;
  std::optional<Hit> dragging_;
  std::optional<Carry> carry_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineDrums2Editor)
};
