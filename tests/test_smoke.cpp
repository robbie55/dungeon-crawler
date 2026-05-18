#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

namespace {
  int Factorial(int n) { return n <= 1 ? n : Factorial(n - 1) * n; }

  TEST_CASE("Testing the Factorial function") {
    CHECK(Factorial(0) == 0);
    CHECK(Factorial(1) == 1);
    CHECK(Factorial(2) == 2);
    CHECK(Factorial(3) == 6);
    CHECK(Factorial(5) == 120);
  }
}  // namespace
