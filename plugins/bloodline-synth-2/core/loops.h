// PROTOTYPE (throwaway spike). Bloodline Synth 2's right panel: two, four or eight bar synth loops,
// bred in a litter of four, with eight keep slots. Each loop has eight traits (0 to 1 between two
// poles) that its notes are drawn from, a seed, the notes themselves, and one kept sound from the
// left panel, chosen by the breeder from the pads' labels. Time locks (ranges of the loop) belong to
// the litter, like Breaks 2's lane locks: notes starting in them pass down unchanged. Plain C++17,
// no JUCE. Times are in beats; 4/4 only.
#pragma once
#include "../../shared/breed/litter.h"
#include <array>
#include <cstdint>
#include <iosfwd>
#include <tuple>
#include <vector>

namespace bs2 {

constexpr int kKitPads = 8, kMaxNotes = 256;
constexpr int kNumBarChoices = 3;
constexpr int kBarChoices[kNumBarChoices] = { 2, 4, 8 };
constexpr double kSixteenth = 0.25;    // beats
constexpr double kMinLength = 1.0 / 16; // beats: 1/64 of a bar

enum Trait { Tonal, Poly, Rhythmic, OffBeat, Dynamic, Open, Dense, Long };
constexpr int kNumTraits = 8;
struct TraitInfo { const char* low; const char* high; };
const TraitInfo& traitInfo(int t);

enum class Scale { Minor, Major, Dorian, Phrygian, MinorPentatonic, MajorPentatonic, Blues, Chromatic };
constexpr int kNumScales = 8;
const char* scaleName(Scale s);
const char* keyName(int key); // 0 is C
bool inScale(int pitch, int key, Scale s);
// "C2": octave n starts at MIDI note 12 * (n + 1), so C3 is 48 and C4 is 60.
constexpr int octaveNote(int octave) { return 12 * (octave + 1); }

struct Note {
  double start = 0, length = 0.25; // beats
  int pitch = 60;
  float velocity = 0.8f;
  bool operator==(const Note& o) const { return std::tie(start, length, pitch, velocity) == std::tie(o.start, o.length, o.pitch, o.velocity); }
};

struct Loop {
  int bars = 2;
  int sound = -1; // the kept pad it plays, or -1
  std::array<double, kNumTraits> traits{};
  std::uint32_t seed = 1;
  std::vector<Note> notes;

  double beats() const { return bars * 4.0; }
  bool operator==(const Loop& o) const { return bars == o.bars && sound == o.sound && traits == o.traits && seed == o.seed && notes == o.notes; }
  bool operator!=(const Loop& o) const { return !(*this == o); }
};

struct Range { double start, end; }; // beats, start inclusive, end exclusive

// The most notes that may sound at once for a Mono / Poly value: 1 at 0, 5 at 1.
int maxVoices(double poly);
// The most notes sounding together anywhere in the loop.
int peakVoices(const Loop&);

// What the litter calls to breed loops, and everything that steers it.
struct LoopOps {
  int bars = 2, key = 0;
  Scale scale = Scale::Minor;
  int octLow = 2, octHigh = 5; // C2 to C5
  std::array<bool, kNumTraits> traitLocks{};
  std::vector<Range> timeLocks;
  std::array<int, kKitPads> pads{ -1, -1, -1, -1, -1, -1, -1, -1 }; // each pad's zone (synth::Zone), or -1

  int lowNote() const { return octaveNote(octLow); }
  int highNote() const { return octaveNote(octHigh); }
  bool lockedAt(double beat, double loopBeats) const;

  Loop fresh(breed::Rng& rng) const;
  Loop mutate(const Loop& from, double spread, breed::Rng& rng) const;
  Loop cross(const Loop& a, const Loop& b, breed::Rng& rng) const; // each bar from one parent

  // Draws the notes again from the loop's traits and its own seed, keeping what is time-locked.
  Loop redraw(const Loop& from) const;
  // The breeder's choice of sound: Bass for low ranges, then by Mono / Poly and Short / Long, then
  // None-labelled sounds, then any. -1 if nothing is kept.
  int chooseSound(const Loop& l, breed::Rng& rng) const;
  bool plays(int sound) const { return sound >= 0 && sound < kKitPads && pads[(size_t) sound] >= 0; }

  // Notes drawn from traits alone, the loop's length, nothing locked.
  std::vector<Note> draw(const std::array<double, kNumTraits>& traits, int bars, std::uint32_t seed) const;
  // Notes starting in a locked range are taken from `from`; nothing else starts there, and notes
  // running into one are trimmed to its edge.
  void applyLocks(Loop& l, const Loop& from) const;
  // Folds pitches into the range, wraps starts into the loop, trims and drops to the voice limit,
  // and sorts. With `respectLocks`, notes starting in a locked range are left exactly as they are.
  void tidy(Loop& l, bool respectLocks) const;
};

// Cuts a loop to fewer bars, or repeats it to fill more.
Loop resize(const Loop& l, int bars);

struct Slot { bool full = false; Loop loop; };

class Loops {
public:
  static constexpr int kLitter = 4, kSlots = 8;

  explicit Loops(std::uint32_t seed = 1);

  const breed::Litter<Loop>& litter() const { return litter_; }
  const LoopOps& ops() const { return ops_; }
  const Slot& slot(int s) const { return slots_[(size_t) s]; }
  int playingSlot() const { return playingSlot_; } // -1 when the heard tile plays
  const Loop& playing() const;

  void select(int i) { litter_.select(i); playingSlot_ = -1; }
  void breedFrom(int i);
  void cross(int i, int j);
  void fresh() { litter_.refresh(ops_); playingSlot_ = -1; }
  bool back() { playingSlot_ = -1; return litter_.back(); }
  void setSpread(double s) { litter_.setSpread(s); }

  void setBars(int bars); // 2, 4 or 8: every candidate is cut or repeated
  void setKey(int key) { ops_.key = ((key % 12) + 12) % 12; }
  void setScale(Scale s) { ops_.scale = s; }
  void setOctaves(int low, int high); // 0 to 8, at least one apart
  void setTraitLock(int t, bool on) { if (t >= 0 && t < kNumTraits) ops_.traitLocks[(size_t) t] = on; }
  void addTimeLock(double start, double end);
  bool removeTimeLockAt(double beat); // false if nothing was locked there
  void clearTimeLocks() { ops_.timeLocks.clear(); }
  void setPads(const std::array<int, kKitPads>& zones);
  // Kit pads a and b swapped places: every loop follows its sound to its new pad.
  void swapPadIndex(int a, int b);

  // By hand, on what is playing.
  void setTrait(int t, double v); // moves the value only
  void redraw();                  // then draws the unlocked notes again from the traits
  int addNote(double start, int pitch); // returns its index, or -1
  void setNote(int i, const Note& n);
  void removeNote(int i);
  void cycleSound();
  void setSound(int pad);

  // Keeping a loop leaves the litter as it is.
  void keep(int tile, int s);
  void swapSlots(int a, int b);
  void clearSlot(int s);
  void playSlot(int s);
  void breedFromSlot(int s);

  void save(std::ostream&) const;
  bool load(std::istream&);

private:
  template <class F> void change(F f);
  void rechooseSilent();

  LoopOps ops_;
  breed::Litter<Loop> litter_;
  std::array<Slot, kSlots> slots_{};
  int playingSlot_ = -1;
};

// Plays a loop: following the host's position on the loop's own length when the host is playing
// (bar 1 of the loop on every 2, 4 or 8 bar boundary), or free-running at the host's tempo from the
// top. Each note-on carries a tag (which engine it went to) and its note-off goes to the same one.
class Player {
public:
  struct Event { int offset, pitch, tag; float velocity; bool on; };

  void prepare(double sampleRate) { sr_ = sampleRate; stop(nullptr); }
  // hostBeat < 0 when the host is stopped. When `run` is false, notes still sounding end at once.
  void advance(const Loop& l, int tag, int n, double bpm, double hostBeat, bool run, std::vector<Event>& out);
  bool running() const { return running_; }
  double beat() const { return beat_; } // within the loop

private:
  struct Active { int pitch, tag; double off; };
  void stop(std::vector<Event>* out);

  double sr_ = 48000.0, free_ = 0.0, last_ = -1.0, beat_ = 0.0;
  bool running_ = false, wasHost_ = false;
  std::vector<Active> active_;
};

} // namespace bs2
