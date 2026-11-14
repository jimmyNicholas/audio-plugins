#include "rack.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace fx {

Rack::Rack(const Chain& chain, double sampleRate, int maxBlock) : chain_(chain) {
  for (int i = 0; i < chain.count; ++i) {
    effects_.push_back(makeEffect(chain.units[i].type));
    effects_.back()->prepare(sampleRate);
  }
  dryL_.assign((size_t) std::max(1, maxBlock), 0.0f);
  dryR_ = dryL_;
}

bool Rack::setSettings(const Chain& chain) {
  if (!chain_.sameShape(chain)) return false;
  chain_ = chain;
  return true;
}

void Rack::process(const float* inL, const float* inR, float* outL, float* outR, int n) {
  std::memcpy(dryL_.data(), inL, sizeof(float) * (size_t) n);
  std::memcpy(dryR_.data(), inR, sizeof(float) * (size_t) n);
  std::memcpy(outL, inL, sizeof(float) * (size_t) n);
  std::memcpy(outR, inR, sizeof(float) * (size_t) n);
  for (size_t i = 0; i < effects_.size(); ++i) effects_[i]->process(outL, outR, n, chain_.units[i].p);
  const float wet = (float) chain_.mix, dry = 1.0f - wet;
  for (int i = 0; i < n; ++i) {
    float l = outL[i] * wet + dryL_[(size_t) i] * dry, r = outR[i] * wet + dryR_[(size_t) i] * dry;
    if (!std::isfinite(l)) l = 0.0f;
    if (!std::isfinite(r)) r = 0.0f;
    outL[i] = l;
    outR[i] = r;
  }
}

void Player::prepare(double sampleRate, int maxBlock) {
  sr_ = sampleRate;
  maxBlock_ = std::max(16, maxBlock);
  fadeStep_ = 1.0 / (0.04 * sr_);
  limitRelease_ = std::exp(-1.0 / (0.08 * sr_));
  for (auto* v : { &inL_, &inR_, &aL_, &aR_, &bL_, &bR_ }) v->assign((size_t) maxBlock_, 0.0f);
  dcX_ = dcY_ = {};
  limitGain_ = 1.0;
}

std::unique_ptr<Rack> Player::use(std::unique_ptr<Rack> rack) {
  std::unique_ptr<Rack> done = std::move(fading_);
  fading_ = std::move(current_);
  current_ = std::move(rack);
  fade_ = fading_ ? 0.0 : 1.0;
  return done;
}

std::array<std::unique_ptr<Rack>, 2> Player::replace(std::unique_ptr<Rack> rack) {
  std::array<std::unique_ptr<Rack>, 2> dropped{ std::move(current_), std::move(fading_) };
  current_ = std::move(rack);
  fade_ = 1.0;
  return dropped;
}

void Player::process(float* left, float* right, int n) {
  for (int at = 0; at < n; at += maxBlock_) block(left + at, right + at, std::min(maxBlock_, n - at));
}

void Player::block(float* l, float* r, int n) {
  std::memcpy(inL_.data(), l, sizeof(float) * (size_t) n);
  std::memcpy(inR_.data(), r, sizeof(float) * (size_t) n);
  if (current_) current_->process(inL_.data(), inR_.data(), aL_.data(), aR_.data(), n);
  else { std::memcpy(aL_.data(), l, sizeof(float) * (size_t) n); std::memcpy(aR_.data(), r, sizeof(float) * (size_t) n); }
  const bool fading = fading_ && fade_ < 1.0;
  if (fading) fading_->process(inL_.data(), inR_.data(), bL_.data(), bR_.data(), n);

  const double wetStep = 1.0 / (0.02 * sr_);
  for (int i = 0; i < n; ++i) {
    double y[2] = { aL_[(size_t) i], aR_[(size_t) i] };
    if (fading) {
      fade_ = std::min(1.0, fade_ + fadeStep_);
      y[0] = bL_[(size_t) i] + (y[0] - bL_[(size_t) i]) * fade_;
      y[1] = bR_[(size_t) i] + (y[1] - bR_[(size_t) i]) * fade_;
    }
    // Bypass fades too, to the untouched input.
    wet_ = std::clamp(wet_ + (bypass_ ? -wetStep : wetStep), 0.0, 1.0);
    const double in[2] = { inL_[(size_t) i], inR_[(size_t) i] };
    for (int c = 0; c < 2; ++c) {
      double v = (in[c] + (y[c] - in[c]) * wet_) * outGain_;
      const double d = v - dcX_[(size_t) c] + 0.9995 * dcY_[(size_t) c];
      dcX_[(size_t) c] = v;
      dcY_[(size_t) c] = d;
      y[c] = std::isfinite(d) ? d : 0.0;
      if (!std::isfinite(d)) dcX_[(size_t) c] = dcY_[(size_t) c] = 0.0;
    }
    // The limiter: instant attack to a -1 dB ceiling, 80 ms release, then a hard stop at full scale.
    const double peak = std::max(std::fabs(y[0]), std::fabs(y[1]));
    const double want = peak > 0.891 ? 0.891 / peak : 1.0;
    limitGain_ = want < limitGain_ ? want : want + (limitGain_ - want) * limitRelease_;
    l[i] = (float) std::clamp(y[0] * limitGain_, -1.0, 1.0);
    r[i] = (float) std::clamp(y[1] * limitGain_, -1.0, 1.0);
  }
  if (fading && fade_ >= 1.0) fade_ = 1.0;
}

} // namespace fx
