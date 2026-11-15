// PROTOTYPE (throwaway spike). Two panels side by side.
//   Left, the sounds: [zone][back][new] / [riff][spread] / [A][B][C][D] / settings / keep pads
//   Right, the loops: [bars][key][scale][back][new] / [playing][midi][octaves][spread] / [A][B][C][D] /
//                     settings (piano roll, lock strip, traits) / keep slots
// Drag a tile onto the keep row below it to keep it; drag a kept one onto another to swap them, or
// back onto its litter to breed from it. A kept sound dragged onto the loop's sound chip sets it.
#pragma once
#include "processor.h"
#include <functional>
#include <optional>
#include <vector>

class BloodlineSynth2Editor : public juce::AudioProcessorEditor, private juce::Timer {
public:
  explicit BloodlineSynth2Editor(BloodlineSynth2Processor&);
  ~BloodlineSynth2Editor() override;

  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;

private:
  enum class Carryable { None, SoundTile, SoundPad, LoopTile, LoopSlot };
  struct Hit {
    Hit(juce::Rectangle<int> a, std::function<void()> c = {}, std::function<void(double)> d = {}, std::function<void()> r = {})
        : area(a), click(std::move(c)), drag(std::move(d)), release(std::move(r)) {}
    juce::Rectangle<int> area;
    std::function<void()> click;
    std::function<void(double)> drag;
    std::function<void()> release; // after a drag ends
    Carryable carry = Carryable::None;
    int index = -1;
    bool midiOut = false; // dragging starts a MIDI file drag into the host
  };
  struct Carry { Carryable what; int index; juce::Point<int> at; bool moving = false; };
  // Editing a note in the big piano roll.
  struct NoteDrag { int index; bool resize, added, moved; bs2::Note orig; juce::Point<int> down; };
  // Dragging out a time lock, or clicking one to unlock it.
  struct StripDrag { double from, to; bool onLock, moved; };

  void timerCallback() override;
  void paintSounds(juce::Graphics&, juce::Rectangle<int> area);
  void paintLoops(juce::Graphics&, juce::Rectangle<int> area);
  void paintSoundTile(juce::Graphics&, int i, juce::Rectangle<int> area);
  void paintLoopTile(juce::Graphics&, int i, juce::Rectangle<int> area);
  void paintSoundSettings(juce::Graphics&, juce::Rectangle<int> area);
  void paintLoopSettings(juce::Graphics&, juce::Rectangle<int> area);
  void paintPads(juce::Graphics&, juce::Rectangle<int> area);
  void paintSlots(juce::Graphics&, juce::Rectangle<int> area);
  void paintPatch(juce::Graphics&, const bs2::Patch&, juce::Rectangle<int> area, float alpha, bool wave);
  void paintRoll(juce::Graphics&, const bs2::Loop&, juce::Rectangle<int> area, bool live, bool big);
  void paintCarry(juce::Graphics&);
  void button(juce::Graphics&, juce::Rectangle<int> r, const juce::String& text, bool on, bool enabled, std::function<void()> click);
  void slider(juce::Graphics&, juce::Rectangle<int> r, double value, juce::Colour c, std::function<void(double)> drag,
              std::function<void()> release = {});
  void label(juce::Graphics&, juce::Rectangle<int> r, const juce::String& text);
  void act(const std::function<void(bs2::Studio&)>& change);
  juce::String soundName(int pad) const;
  int indexAt(const std::vector<juce::Rectangle<int>>&, juce::Point<int>) const;

  // The big roll's geometry, from the last paint.
  double beatAt(int x) const;
  int pitchAt(int y) const;
  bool rollMouseDown(const juce::MouseEvent&);
  bool stripMouseDown(const juce::MouseEvent&);

  BloodlineSynth2Processor& proc_;
  BloodlineSynth2Processor::View view_;
  std::vector<Hit> hits_;
  std::vector<juce::Rectangle<int>> padAreas_, slotAreas_;
  std::vector<std::pair<juce::Rectangle<float>, int>> noteRects_;
  juce::Rectangle<int> soundLitter_, loopLitter_, soundChip_, roll_, strip_;
  std::optional<Hit> dragging_;
  std::optional<Carry> carry_;
  std::optional<NoteDrag> noteDrag_;
  std::optional<StripDrag> stripDrag_;
  int octHandle_ = -1;
  bool midiDragStarted_ = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BloodlineSynth2Editor)
};
