// PROTOTYPE (throwaway spike). Mouse led, laid out like Bloodline Synth: a litter of eight one-bar
// tiles (click to hear it loop, Breed, Cross), the style presets, swing dial and pull above, the
// taste below, and an inspector with the heard bar's grid, lane roles and locks, and a MIDI file
// to drag into Live.
#pragma once
#include "processor.h"
#include <functional>
#include <optional>
#include <vector>

class BloodlineBreaksEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
  explicit BloodlineBreaksEditor(BloodlineBreaksProcessor&);
  ~BloodlineBreaksEditor() override;

  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;

private:
  struct Hit {
    juce::Rectangle<int> area;
    std::function<void()> click;
    std::function<void(double)> drag;
    bool midiOut = false; // dragging starts a MIDI file drag into the host
  };

  void timerCallback() override;
  void paintTile(juce::Graphics&, int i, juce::Rectangle<int> area);
  void paintInspector(juce::Graphics&, juce::Rectangle<int> area);
  void paintTaste(juce::Graphics&, juce::Rectangle<int> area);
  void paintGrid(juce::Graphics&, const breaks::Pattern&, juce::Rectangle<int> area, bool heard, bool big);
  void slider(juce::Graphics&, juce::Rectangle<int> r, double value, juce::Colour c, std::function<void(double)> drag);
  void button(juce::Graphics&, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled,
              std::function<void()> click);
  void act(const std::function<void(breaks::Breeder&)>& change);

  BloodlineBreaksProcessor& proc_;
  BloodlineBreaksProcessor::View view_;
  std::vector<Hit> hits_;
  std::optional<Hit> dragging_;
  bool midiDragStarted_ = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineBreaksEditor)
};
