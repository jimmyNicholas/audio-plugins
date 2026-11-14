// The second spike's UI (#38, #44, #45): the 4x4 grid as a workbench under a kit of three slots,
// each line with a tab and its on-screen actions, and one inspector for the selected line: zone,
// spread, the row's Pan, Level and Mono, then the five collapsible setting groups, each setting a
// slider with a lock.
#pragma once
#include "processor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <functional>
#include <vector>

class BloodlineEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
  explicit BloodlineEditor(BloodlineProcessor&);
  ~BloodlineEditor() override;

  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;

private:
  // Something clickable, laid out while painting: a click, or a drag along it (0 to 1).
  struct Hit {
    juce::Rectangle<int> area;
    std::function<void()> click;
    std::function<void(double)> drag;
  };

  void timerCallback() override;
  void paintLine(juce::Graphics&, int line, juce::Rectangle<int> area);
  void paintPad(juce::Graphics&, int note, juce::Rectangle<int> area);
  void paintInspector(juce::Graphics&, juce::Rectangle<int> area);
  void button(juce::Graphics&, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled,
              juce::Colour c, std::function<void()> click);
  void slider(juce::Graphics&, juce::Rectangle<int> track, double value, bool enabled, juce::Colour c,
              std::function<void(double)> drag);
  void act(const std::function<void(bloodline::Instrument&)>& change);

  BloodlineProcessor& proc_;
  BloodlineProcessor::View view_;
  int selected_ = bloodline::kBench;
  std::vector<Hit> hits_;
  std::optional<Hit> dragging_;
  juce::uint32 confirmUntil_ = 0; // Start again asks for a second click

  std::array<int, bloodline::kNumPads> seenHits_{};
  std::array<juce::uint32, bloodline::kNumPads> flashUntil_{};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineEditor)
};
