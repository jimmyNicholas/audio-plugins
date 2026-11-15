// A tiny test harness: no dependencies, one executable per seam, run by ctest.
#pragma once
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace check {

struct Case { const char* name; std::function<void()> body; };
inline std::vector<Case>& cases() { static std::vector<Case> c; return c; }
inline int& failures() { static int f = 0; return f; }
inline bool& caseFailed() { static bool f = false; return f; }

struct Register { Register(const char* n, std::function<void()> b) { cases().push_back({ n, b }); } };

inline void fail(const char* file, int line, const std::string& what) {
  std::printf("    %s:%d: %s\n", file, line, what.c_str());
  caseFailed() = true;
}

inline int run() {
  for (auto& c : cases()) {
    caseFailed() = false;
    c.body();
    std::printf("%s %s\n", caseFailed() ? "FAIL" : "ok  ", c.name);
    if (caseFailed()) failures() += 1;
  }
  std::printf("%d of %d failed\n", failures(), (int) cases().size());
  return failures() == 0 ? 0 : 1;
}

} // namespace check

#define CHECK_CAT2(a, b) a##b
#define CHECK_CAT(a, b) CHECK_CAT2(a, b)
#define TEST(name) \
  static void CHECK_CAT(test_, __LINE__)(); \
  static check::Register CHECK_CAT(reg_, __LINE__)(name, CHECK_CAT(test_, __LINE__)); \
  static void CHECK_CAT(test_, __LINE__)()

#define CHECK(cond) do { if (!(cond)) check::fail(__FILE__, __LINE__, "CHECK(" #cond ")"); } while (0)
#define CHECK_EQ(a, b) do { auto _a = (a); auto _b = (b); if (!(_a == _b)) \
  check::fail(__FILE__, __LINE__, std::string(#a " == " #b ": ") + std::to_string(_a) + " vs " + std::to_string(_b)); } while (0)
#define CHECK_NEAR(a, b, tol) do { double _a = (a), _b = (b); if (!(std::fabs(_a - _b) <= (tol))) \
  check::fail(__FILE__, __LINE__, std::string(#a " ~ " #b ": ") + std::to_string(_a) + " vs " + std::to_string(_b)); } while (0)

#define CHECK_MAIN int main() { return check::run(); }
