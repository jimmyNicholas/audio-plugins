// Layout C from #38: a bare 4x4 pad grid with a tab beside each breeding row, and one inspector
// for the selected row (zone, spread, and each setting with its lock).
#pragma once
#include "processor.h"
#include <juce_audio_utils/juce_audio_utils.h>

class BloodlineEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
  explicit BloodlineEditor(BloodlineProcessor&);
  ~BloodlineEditor() override;

  void paint(juce::Graphics&) override;
  void resized() override;
  void mouseDown(const juce::MouseEvent&) override;

private:
  void timerCallback() override;
  void paintPad(juce::Graphics&, int note);
  void paintInspector(juce::Graphics&);

  BloodlineProcessor& proc_;
  std::array<bloodline::Row, bloodline::kNumRows> rows_;
  int selected_ = 2; // the Kick row, nearest the bottom pads

  std::array<juce::Rectangle<int>, bloodline::kNumPads> padRects_;
  std::array<juce::Rectangle<int>, bloodline::kNumRows> tabRects_;
  std::array<juce::Rectangle<int>, bloodline::kNumZones> zoneRects_;
  std::array<juce::Rectangle<int>, bloodline::kNumSettings> lockRects_, trackRects_;
  juce::Rectangle<int> inspector_, spreadRect_;

  std::array<int, bloodline::kNumPads> seenHits_{};
  std::array<juce::uint32, bloodline::kNumPads> flashUntil_{};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineEditor)
};
