// PROTOTYPE (throwaway spike). Two panels side by side.
//   Left, the drums:  [zone][back][new] / [loop][spread] / [A][B][C][D] / settings / keep pads
//   Right, the loops: [style][back][new] / [playing][midi][swing][pull] / [A][B][C][D] / settings / keep slots
// Drag a tile onto the keep row below it to keep it; drag a kept one onto another to swap them,
// or back onto its litter to breed from it.
#pragma once
#include "processor.h"
#include <functional>
#include <optional>
#include <vector>

class BloodlineBreaks2Editor : public juce::AudioProcessorEditor, private juce::Timer {
public:
  explicit BloodlineBreaks2Editor(BloodlineBreaks2Processor&);
  ~BloodlineBreaks2Editor() override;

  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;

private:
  enum class Carryable { None, DrumTile, DrumPad, LoopTile, LoopSlot };
  struct Hit {
    Hit(juce::Rectangle<int> a, std::function<void()> c = {}, std::function<void(double)> d = {})
        : area(a), click(std::move(c)), drag(std::move(d)) {}
    juce::Rectangle<int> area;
    std::function<void()> click;
    std::function<void(double)> drag;
    Carryable carry = Carryable::None;
    int index = -1;
    bool midiOut = false; // dragging starts a MIDI file drag into the host
  };
  struct Carry { Carryable what; int index; juce::Point<int> at; bool moving = false; };

  void timerCallback() override;
  void paintDrums(juce::Graphics&, juce::Rectangle<int> area);
  void paintLoops(juce::Graphics&, juce::Rectangle<int> area);
  void paintDrumTile(juce::Graphics&, int i, juce::Rectangle<int> area);
  void paintLoopTile(juce::Graphics&, int i, juce::Rectangle<int> area);
  void paintDrumSettings(juce::Graphics&, juce::Rectangle<int> area);
  void paintLoopSettings(juce::Graphics&, juce::Rectangle<int> area);
  void paintPads(juce::Graphics&, juce::Rectangle<int> area);
  void paintSlots(juce::Graphics&, juce::Rectangle<int> area);
  void paintSound(juce::Graphics&, const bb2::Values&, juce::Rectangle<int> area, float alpha);
  void paintGrid(juce::Graphics&, const bb2::Pattern&, juce::Rectangle<int> area, bool live, bool big);
  void paintCarry(juce::Graphics&);
  void button(juce::Graphics&, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled, std::function<void()> click);
  void slider(juce::Graphics&, juce::Rectangle<int> r, double value, juce::Colour c, std::function<void(double)> drag);
  void label(juce::Graphics&, juce::Rectangle<int> r, const juce::String& text);
  void act(const std::function<void(bb2::Studio&)>& change);
  juce::String soundName(std::uint8_t mask) const;
  int indexAt(const std::vector<juce::Rectangle<int>>&, juce::Point<int>) const;

  BloodlineBreaks2Processor& proc_;
  BloodlineBreaks2Processor::View view_;
  std::vector<Hit> hits_;
  std::vector<juce::Rectangle<int>> padAreas_, slotAreas_;
  juce::Rectangle<int> drumLitter_, loopLitter_;
  std::optional<Hit> dragging_;
  std::optional<Carry> carry_;
  bool midiDragStarted_ = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineBreaks2Editor)
};
