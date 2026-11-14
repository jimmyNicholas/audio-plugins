#include "effects.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace fx {

namespace {

constexpr double kPi = 3.14159265358979323846;

double lin(double n, double a, double b) { return a + (b - a) * n; }
double expo(double n, double a, double b) { return a * std::pow(b / a, n); }
double onePole(double hz, double sr) { return 1.0 - std::exp(-2.0 * kPi * std::min(hz, 0.45 * sr) / sr); }
double coefMs(double ms, double sr) { return std::exp(-1.0 / std::max(1.0, ms * 0.001 * sr)); }

// A fractional delay line.
struct Line {
  std::vector<float> buf;
  int pos = 0;
  void size(int n) { buf.assign((size_t) std::max(4, n), 0.0f); pos = 0; }
  void push(float x) { buf[(size_t) pos] = x; pos = (pos + 1) % (int) buf.size(); }
  float at(double delaySamples) const { // delaySamples back from the newest
    const int n = (int) buf.size();
    delaySamples = std::clamp(delaySamples, 1.0, (double) n - 2);
    const double r = pos - 1 - delaySamples;
    const double rf = std::floor(r);
    const int i0 = ((int) rf % n + n) % n, i1 = (i0 + 1) % n;
    const float f = (float) (r - rf);
    return buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * f;
  }
};

// Simper's state-variable filter; one per channel.
struct Svf {
  double ic1 = 0, ic2 = 0;
  void run(double x, double g, double k, double& low, double& band, double& high) {
    const double a1 = 1.0 / (1.0 + g * (g + k)), a2 = g * a1, a3 = g * a2;
    const double v3 = x - ic2, v1 = a1 * ic1 + a2 * v3, v2 = ic2 + a2 * ic1 + a3 * v3;
    ic1 = 2 * v1 - ic1;
    ic2 = 2 * v2 - ic2;
    low = v2; band = v1; high = x - k * v1 - v2;
    if (!std::isfinite(ic1) || !std::isfinite(ic2)) ic1 = ic2 = 0;
  }
};

struct Rand {
  std::uint32_t s = 0x1234567u;
  double next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s / 4294967296.0; }
};

// ---------------------------------------------------------------------------------------------

class Drive : public Effect {
public:
  void prepare(double sr) override { sr_ = sr; lp_ = { 0, 0 }; dc_ = { 0, 0 }; dcx_ = { 0, 0 }; }
  void process(float* l, float* r, int n, const std::array<double, kParams>& p) override {
    const double gain = expo(p[0], 1.0, 60.0), fold = p[1], bias = lin(p[3], -0.6, 0.6);
    const double a = onePole(expo(p[2], 400.0, 20000.0), sr_);
    const double norm = 1.0 / std::tanh(gain * 0.5 + 0.5);
    float* ch[2] = { l, r };
    for (int c = 0; c < 2; ++c)
      for (int i = 0; i < n; ++i) {
        const double x = ch[c][i] * gain + bias;
        const double soft = std::tanh(x) * norm;
        const double folded = std::sin(x * kPi * 0.5);
        double y = soft + (folded - soft) * fold;
        lp_[c] += (y - lp_[c]) * a;
        y = lp_[c];
        const double d = y - dcx_[c] + 0.995 * dc_[c]; // the bias leaves a DC offset; take it out
        dcx_[c] = y;
        dc_[c] = d;
        ch[c][i] = (float) d;
      }
  }
private:
  double sr_ = 48000;
  std::array<double, 2> lp_{}, dc_{}, dcx_{};
};

class Crush : public Effect {
public:
  void prepare(double sr) override { sr_ = sr; held_ = { 0, 0 }; phase_ = 1; }
  void process(float* l, float* r, int n, const std::array<double, kParams>& p) override {
    const double steps = std::pow(2.0, lin(p[0], 16.0, 1.5) - 1.0);
    const double rate = expo(p[1], 48000.0, 200.0), jitter = p[2], mix = p[3];
    for (int i = 0; i < n; ++i) {
      phase_ += rate / sr_ * (1.0 + jitter * (rnd_.next() * 2.0 - 1.0));
      if (phase_ >= 1.0) {
        phase_ -= std::floor(phase_);
        held_[0] = std::round(l[i] * steps) / steps;
        held_[1] = std::round(r[i] * steps) / steps;
      }
      l[i] = (float) (l[i] + (held_[0] - l[i]) * mix);
      r[i] = (float) (r[i] + (held_[1] - r[i]) * mix);
    }
  }
private:
  double sr_ = 48000, phase_ = 1;
  std::array<double, 2> held_{};
  Rand rnd_;
};

class Filter : public Effect {
public:
  void prepare(double sr) override { sr_ = sr; f_ = {}; lfo_ = 0; }
  void process(float* l, float* r, int n, const std::array<double, kParams>& p) override {
    const double cutoff = expo(p[0], 30.0, 18000.0), k = 2.0 - 2.0 * lin(p[1], 0.0, 0.98), mode = p[2];
    const double depth = p[3] * 4.0, rate = expo(p[3], 0.1, 8.0);
    float* ch[2] = { l, r };
    for (int i = 0; i < n; ++i) {
      if (i % 16 == 0) {
        const double hz = cutoff * std::exp2(depth * std::sin(2 * kPi * lfo_));
        g_ = std::tan(kPi * std::clamp(hz, 20.0, 0.45 * sr_) / sr_);
      }
      lfo_ += rate / sr_;
      lfo_ -= std::floor(lfo_);
      for (int c = 0; c < 2; ++c) {
        double lo, band, hi;
        f_[c].run(ch[c][i], g_, k, lo, band, hi);
        // Low-pass, then band-pass, then high-pass, morphing between neighbours.
        const double y = mode < 0.5 ? lo + (band * 1.5 - lo) * (mode * 2) : band * 1.5 + (hi - band * 1.5) * (mode * 2 - 1);
        ch[c][i] = (float) y;
      }
    }
  }
private:
  double sr_ = 48000, lfo_ = 0, g_ = 0.1;
  std::array<Svf, 2> f_{};
};

class Comb : public Effect {
public:
  void prepare(double sr) override { sr_ = sr; for (auto& d : d_) d.size((int) (0.06 * sr)); lfo_ = 0; }
  void process(float* l, float* r, int n, const std::array<double, kParams>& p) override {
    const double ms = expo(p[0], 0.2, 25.0), fb = lin(p[1], -0.97, 0.97), depth = p[2] * 0.5, rate = expo(p[3], 0.05, 8.0);
    float* ch[2] = { l, r };
    for (int i = 0; i < n; ++i) {
      lfo_ += rate / sr_;
      lfo_ -= std::floor(lfo_);
      for (int c = 0; c < 2; ++c) {
        const double mod = 1.0 + depth * std::sin(2 * kPi * (lfo_ + c * 0.25));
        const double y = d_[c].at(ms * 0.001 * sr_ * mod);
        d_[c].push((float) std::tanh(ch[c][i] + y * fb));
        ch[c][i] = (float) (0.5 * ch[c][i] + 0.5 * y);
      }
    }
  }
private:
  double sr_ = 48000, lfo_ = 0;
  std::array<Line, 2> d_;
};

class Delay : public Effect {
public:
  void prepare(double sr) override { sr_ = sr; for (auto& d : d_) d.size((int) (1.6 * sr)); lp_ = { 0, 0 }; }
  void process(float* l, float* r, int n, const std::array<double, kParams>& p) override {
    const double t = expo(p[0], 0.01, 1.5) * sr_, fb = lin(p[1], 0.0, 1.1), a = onePole(expo(p[2], 300.0, 18000.0), sr_);
    const double ping = p[3];
    for (int i = 0; i < n; ++i) {
      double y[2] = { d_[0].at(t), d_[1].at(t) };
      for (int c = 0; c < 2; ++c) { lp_[c] += (y[c] - lp_[c]) * a; y[c] = lp_[c]; }
      // Ping-pong crosses each side's echo into the other.
      const double in[2] = { l[i], r[i] };
      for (int c = 0; c < 2; ++c) {
        // Full ping-pong sends the input in on the left only and crosses every echo over.
        const double feed = in[c] * (1 - ping) + (c == 0 ? 0.5 * (in[0] + in[1]) * ping : 0.0);
        const double back = y[c] * (1 - ping) + y[1 - c] * ping;
        d_[c].push((float) std::tanh(feed + back * fb)); // tanh keeps feedback above 1 from running away
      }
      l[i] = (float) (in[0] + 0.7 * y[0]);
      r[i] = (float) (in[1] + 0.7 * y[1]);
    }
  }
private:
  double sr_ = 48000;
  std::array<Line, 2> d_;
  std::array<double, 2> lp_{};
};

// Schroeder: four combs and two all-passes a side, the right side a little longer for width.
class Reverb : public Effect {
public:
  void prepare(double sr) override {
    sr_ = sr;
    const double scale = sr / 44100.0 * 2.0; // room for the largest Size
    for (int c = 0; c < 2; ++c) {
      for (int k = 0; k < 4; ++k) combs_[c][k].size((int) ((kComb[k] + c * 23) * scale) + 8);
      for (int k = 0; k < 2; ++k) aps_[c][k].size((int) ((kAp[k] + c * 23) * sr / 44100.0) + 8);
    }
    damp_ = {};
  }
  void process(float* l, float* r, int n, const std::array<double, kParams>& p) override {
    const double size = lin(p[0], 0.3, 2.0) * sr_ / 44100.0;
    // At the top of Decay the tail freezes: it stops taking input and holds what it has.
    const bool frozen = p[1] > 0.97;
    const double fb = frozen ? 0.9995 : lin(p[1], 0.7, 0.985), damp = frozen ? 0.0 : lin(p[2], 0.0, 0.9), mix = p[3];
    float* ch[2] = { l, r };
    for (int i = 0; i < n; ++i) {
      const double in = frozen ? 0.0 : 0.5 * (l[i] + r[i]) * 0.25;
      for (int c = 0; c < 2; ++c) {
        double sum = 0;
        for (int k = 0; k < 4; ++k) {
          const double y = combs_[c][k].at((kComb[k] + c * 23) * size);
          damp_[c][k] = y + (damp_[c][k] - y) * damp;
          combs_[c][k].push((float) std::tanh(in + damp_[c][k] * fb));
          sum += y;
        }
        for (int k = 0; k < 2; ++k) {
          const double d = (kAp[k] + c * 23) * sr_ / 44100.0;
          const double y = aps_[c][k].at(d);
          aps_[c][k].push((float) (sum + y * 0.5));
          sum = y - sum * 0.5;
        }
        ch[c][i] = (float) (ch[c][i] + sum * mix);
      }
    }
  }
private:
  static constexpr double kComb[4] = { 1116, 1188, 1277, 1356 }, kAp[2] = { 556, 441 };
  double sr_ = 48000;
  std::array<std::array<Line, 4>, 2> combs_;
  std::array<std::array<Line, 2>, 2> aps_;
  std::array<std::array<double, 4>, 2> damp_{};
};

class Gate : public Effect {
public:
  void prepare(double sr) override { sr_ = sr; phase_ = 0; env_ = 1; }
  void process(float* l, float* r, int n, const std::array<double, kParams>& p) override {
    const double rate = expo(p[0], 0.5, 24.0), duty = lin(p[1], 0.05, 0.95), depth = p[2];
    const double c = coefMs(expo(p[3], 0.5, 30.0), sr_);
    for (int i = 0; i < n; ++i) {
      phase_ += rate / sr_;
      phase_ -= std::floor(phase_);
      const double target = phase_ < duty ? 1.0 : 1.0 - depth;
      env_ = target + (env_ - target) * c;
      l[i] = (float) (l[i] * env_);
      r[i] = (float) (r[i] * env_);
    }
  }
private:
  double sr_ = 48000, phase_ = 0, env_ = 1;
};

class Ring : public Effect {
public:
  void prepare(double sr) override { sr_ = sr; phase_ = drift_ = 0; }
  void process(float* l, float* r, int n, const std::array<double, kParams>& p) override {
    const double hz = expo(p[0], 5.0, 5000.0), shape = p[1], drift = p[2] * 2.0, mix = p[3];
    for (int i = 0; i < n; ++i) {
      drift_ += 0.3 / sr_;
      drift_ -= std::floor(drift_);
      phase_ += hz * std::exp2(drift * std::sin(2 * kPi * drift_)) / sr_;
      phase_ -= std::floor(phase_);
      const double s = std::sin(2 * kPi * phase_);
      const double carrier = s + ((s >= 0 ? 1.0 : -1.0) - s) * shape;
      l[i] = (float) (l[i] * (1 - mix + mix * carrier));
      r[i] = (float) (r[i] * (1 - mix + mix * carrier));
    }
  }
private:
  double sr_ = 48000, phase_ = 0, drift_ = 0;
};

class Tape : public Effect {
public:
  void prepare(double sr) override { sr_ = sr; for (auto& d : d_) d.size((int) (0.06 * sr)); wow_ = flutter_ = 0; }
  void process(float* l, float* r, int n, const std::array<double, kParams>& p) override {
    const double wowMs = p[0] * 15.0, wowHz = expo(p[1], 0.1, 4.0), flutterMs = p[2] * 2.0, sat = expo(p[3], 1.0, 10.0);
    const double norm = 1.0 / std::sqrt(sat); // saturates without getting much louder
    float* ch[2] = { l, r };
    for (int i = 0; i < n; ++i) {
      wow_ += wowHz / sr_; wow_ -= std::floor(wow_);
      flutter_ += 11.0 / sr_; flutter_ -= std::floor(flutter_);
      const double ms = 20.0 + wowMs * std::sin(2 * kPi * wow_) + flutterMs * std::sin(2 * kPi * flutter_);
      for (int c = 0; c < 2; ++c) {
        d_[c].push(ch[c][i]);
        ch[c][i] = (float) (std::tanh(d_[c].at(ms * 0.001 * sr_) * sat) * norm);
      }
    }
  }
private:
  double sr_ = 48000, wow_ = 0, flutter_ = 0;
  std::array<Line, 2> d_;
};

class Comp : public Effect {
public:
  void prepare(double sr) override { sr_ = sr; env_ = 0; }
  void process(float* l, float* r, int n, const std::array<double, kParams>& p) override {
    const double thresh = lin(p[0], -50.0, 0.0), ratio = expo(p[1], 1.0, 30.0);
    const double att = coefMs(expo(p[2], 0.1, 100.0), sr_), rel = coefMs(expo(p[3], 10.0, 1500.0), sr_);
    const double makeup = std::pow(10.0, -thresh * (1.0 - 1.0 / ratio) * 0.5 / 20.0);
    for (int i = 0; i < n; ++i) {
      const double level = std::max(std::fabs(l[i]), std::fabs(r[i]));
      env_ = level > env_ ? level + (env_ - level) * att : level + (env_ - level) * rel;
      const double db = 20.0 * std::log10(std::max(env_, 1e-6));
      const double over = std::max(0.0, db - thresh);
      const double gain = std::pow(10.0, -over * (1.0 - 1.0 / ratio) / 20.0) * makeup;
      l[i] = (float) (l[i] * gain);
      r[i] = (float) (r[i] * gain);
    }
  }
private:
  double sr_ = 48000, env_ = 0;
};

class Width : public Effect {
public:
  void prepare(double sr) override { sr_ = sr; d_.size((int) (0.035 * sr)); spin_ = 0; }
  void process(float* l, float* r, int n, const std::array<double, kParams>& p) override {
    const double width = p[0] * 2.0, haas = p[1] * 30.0 * 0.001 * sr_, rate = p[2] < 0.05 ? 0.0 : expo(p[2], 0.05, 8.0), depth = p[3];
    for (int i = 0; i < n; ++i) {
      const double mid = 0.5 * (l[i] + r[i]), side = 0.5 * (l[i] - r[i]) * width;
      double a = mid + side, b = mid - side;
      d_.push((float) b);
      if (haas > 1.0) b = d_.at(haas);
      spin_ += rate / sr_;
      spin_ -= std::floor(spin_);
      const double pan = depth * std::sin(2 * kPi * spin_); // -1 left to 1 right
      a *= std::min(1.0, 1.0 - pan);
      b *= std::min(1.0, 1.0 + pan);
      l[i] = (float) a;
      r[i] = (float) b;
    }
  }
private:
  double sr_ = 48000, spin_ = 0;
  Line d_;
};

// Keeps recording; now and then it grabs the last slice and repeats it, at a speed (backwards
// below zero), with short fades so the joins do not click.
class Glitch : public Effect {
public:
  void prepare(double sr) override {
    sr_ = sr;
    for (auto& b : buf_) b.assign((size_t) (2.1 * sr), 0.0f);
    write_ = 0; countdown_ = 0; repeating_ = false; fade_ = 0;
  }
  void process(float* l, float* r, int n, const std::array<double, kParams>& p) override {
    const double chance = p[0], len = expo(p[1], 0.01, 0.5) * sr_, mix = p[3];
    double speed = lin(p[2], -2.0, 2.0);
    if (std::fabs(speed) < 0.25) speed = speed < 0 ? -0.25 : 0.25;
    const int size = (int) buf_[0].size();
    const double fadeStep = 1.0 / (0.003 * sr_);
    for (int i = 0; i < n; ++i) {
      buf_[0][(size_t) write_] = l[i];
      buf_[1][(size_t) write_] = r[i];
      write_ = (write_ + 1) % size;
      if (--countdown_ <= 0) { // a decision every slice
        countdown_ = (int) len;
        repeating_ = rnd_.next() < chance;
        if (repeating_ && !wasRepeating_) { start_ = write_ - len; read_ = speed < 0 ? len : 0; }
        wasRepeating_ = repeating_;
        sliceLen_ = len; speed_ = speed;
      }
      fade_ = std::clamp(fade_ + (repeating_ ? fadeStep : -fadeStep), 0.0, 1.0);
      if (fade_ > 0.0) {
        read_ += speed_;
        if (read_ >= sliceLen_) read_ -= sliceLen_;
        if (read_ < 0) read_ += sliceLen_;
        const double pos = start_ + read_;
        const int i0 = (((int) std::floor(pos)) % size + size) % size;
        const float g = (float) (fade_ * mix);
        l[i] = l[i] * (1 - g) + buf_[0][(size_t) i0] * g;
        r[i] = r[i] * (1 - g) + buf_[1][(size_t) i0] * g;
      }
    }
  }
private:
  double sr_ = 48000, start_ = 0, read_ = 0, sliceLen_ = 1, speed_ = 1, fade_ = 0;
  int write_ = 0, countdown_ = 0;
  bool repeating_ = false, wasRepeating_ = false;
  std::array<std::vector<float>, 2> buf_;
  Rand rnd_;
};

} // namespace

std::unique_ptr<Effect> makeEffect(Type t) {
  switch (t) {
    case Type::Drive: return std::make_unique<Drive>();
    case Type::Crush: return std::make_unique<Crush>();
    case Type::Filter: return std::make_unique<Filter>();
    case Type::Comb: return std::make_unique<Comb>();
    case Type::Delay: return std::make_unique<Delay>();
    case Type::Reverb: return std::make_unique<Reverb>();
    case Type::Gate: return std::make_unique<Gate>();
    case Type::Ring: return std::make_unique<Ring>();
    case Type::Tape: return std::make_unique<Tape>();
    case Type::Comp: return std::make_unique<Comp>();
    case Type::Width: return std::make_unique<Width>();
    case Type::Glitch: return std::make_unique<Glitch>();
  }
  return std::make_unique<Drive>();
}

const char* formatSetting(Type t, int k, double v, char* b, int size) {
  auto pct = [&](double x) { std::snprintf(b, (size_t) size, "%.0f%%", x * 100); };
  auto hz = [&](double x) { if (x >= 1000) std::snprintf(b, (size_t) size, "%.1f kHz", x / 1000); else std::snprintf(b, (size_t) size, "%.*f Hz", x < 10 ? 2 : 0, x); };
  auto ms = [&](double s) { if (s >= 1) std::snprintf(b, (size_t) size, "%.2f s", s); else std::snprintf(b, (size_t) size, "%.*f ms", s < 0.01 ? 1 : 0, s * 1000); };
  switch (t) {
    case Type::Drive: if (k == 0) std::snprintf(b, (size_t) size, "x%.1f", expo(v, 1, 60)); else if (k == 2) hz(expo(v, 400, 20000)); else if (k == 3) std::snprintf(b, (size_t) size, "%+.2f", lin(v, -0.6, 0.6)); else pct(v); break;
    case Type::Crush: if (k == 0) std::snprintf(b, (size_t) size, "%.1f bits", lin(v, 16, 1.5)); else if (k == 1) hz(expo(v, 48000, 200)); else pct(v); break;
    case Type::Filter: if (k == 0) hz(expo(v, 30, 18000)); else if (k == 2) std::snprintf(b, (size_t) size, "%s", v < 0.25 ? "low-pass" : v < 0.75 ? "band-pass" : "high-pass"); else if (k == 3) std::snprintf(b, (size_t) size, "%.1f oct", v * 4); else pct(v); break;
    case Type::Comb: if (k == 0) ms(expo(v, 0.2, 25) / 1000); else if (k == 1) std::snprintf(b, (size_t) size, "%+.0f%%", lin(v, -97, 97)); else if (k == 3) hz(expo(v, 0.05, 8)); else pct(v); break;
    case Type::Delay: if (k == 0) ms(expo(v, 0.01, 1.5)); else if (k == 1) pct(lin(v, 0, 1.1)); else if (k == 2) hz(expo(v, 300, 18000)); else pct(v); break;
    case Type::Reverb: if (k == 0) std::snprintf(b, (size_t) size, "x%.2f", lin(v, 0.3, 2)); else if (k == 1 && v > 0.97) std::snprintf(b, (size_t) size, "frozen"); else pct(v); break;
    case Type::Gate: if (k == 0) hz(expo(v, 0.5, 24)); else if (k == 1) pct(lin(v, 0.05, 0.95)); else if (k == 3) ms(expo(v, 0.5, 30) / 1000); else pct(v); break;
    case Type::Ring: if (k == 0) hz(expo(v, 5, 5000)); else if (k == 2) std::snprintf(b, (size_t) size, "%.1f oct", v * 2); else pct(v); break;
    case Type::Tape: if (k == 0) ms(v * 0.015); else if (k == 1) hz(expo(v, 0.1, 4)); else if (k == 2) ms(v * 0.002); else std::snprintf(b, (size_t) size, "x%.1f", expo(v, 1, 10)); break;
    case Type::Comp: if (k == 0) std::snprintf(b, (size_t) size, "%.0f dB", lin(v, -50, 0)); else if (k == 1) std::snprintf(b, (size_t) size, "%.1f:1", expo(v, 1, 30)); else if (k == 2) ms(expo(v, 0.1, 100) / 1000); else ms(expo(v, 10, 1500) / 1000); break;
    case Type::Width: if (k == 0) pct(v * 2); else if (k == 1) ms(v * 0.03); else if (k == 2) { if (v < 0.05) std::snprintf(b, (size_t) size, "off"); else hz(expo(v, 0.05, 8)); } else pct(v); break;
    case Type::Glitch: if (k == 1) ms(expo(v, 0.01, 0.5)); else if (k == 2) std::snprintf(b, (size_t) size, "x%+.2f", lin(v, -2, 2)); else pct(v); break;
  }
  return b;
}

} // namespace fx
