// The twelve effects, each a stereo in-place processor reading four normalised settings. Buffers
// are allocated in prepare(), never while processing. Ranges are wide on purpose. Plain C++17.
#pragma once
#include "chain.h"
#include <array>
#include <memory>

namespace fx {

class Effect {
public:
  virtual ~Effect() = default;
  virtual void prepare(double sampleRate) = 0;
  // Processes n samples of left and right in place.
  virtual void process(float* left, float* right, int n, const std::array<double, kParams>& p) = 0;
};

std::unique_ptr<Effect> makeEffect(Type t);

// Real-unit readouts of a unit's settings, for the UI.
const char* formatSetting(Type t, int param, double norm, char* buffer, int size);

} // namespace fx
