#include <doctest/doctest.h>

#include <cstddef>
#include <memory>
#include <optional>

#include "core/states.hpp"

namespace {

  // Victory is the only stub whose Tick() carries real logic, so it is the only one worth
  // a test: it holds for kVictoryFrames, then asks the FSM to swap itself out for Hub. The
  // other states' Ticks return nullopt unconditionally; testing those would only pin down
  // the placeholder (see TODO.md "Explicitly NOT tested").
  TEST_CASE("Victory::Tick holds for kVictoryFrames, then asks to replace itself with Hub") {
    states::Victory victory;

    // The hold: the first kVictoryFrames ticks request no transition.
    for (std::size_t frame{0}; frame < states::kVictoryFrames; ++frame) {
      CHECK_FALSE(victory.Tick().has_value());
    }

    // The next tick crosses the threshold and routes a kReplace -> Hub through the FSM.
    const std::optional<states::StateRequest> kRequest{victory.Tick()};
    REQUIRE(kRequest.has_value());
    CHECK(kRequest->type == states::RequestType::kReplace);
    REQUIRE(kRequest->payload != nullptr);
    CHECK(dynamic_cast<states::Hub*>(kRequest->payload.get()) != nullptr);
  }

}  // namespace
