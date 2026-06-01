#include <doctest/doctest.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "core/fsm_helpers.hpp"
#include "core/state_fsm.hpp"
#include "core/states.hpp"

namespace {

  // A sink the doubles report into. It lives as a local in each test and is injected into
  // every TestState by pointer; declared before the StateFSM, it outlives the FSM that
  // owns those doubles, so the destructor callbacks fired during teardown still have a
  // valid target. Injecting a tracker (rather than static counters on the double) keeps
  // the double free of the non-const global the linter forbids and keeps cases isolated.
  struct Tracker {
    int constructed{0};
    int destroyed{0};
    std::vector<int> tick_order;    // ids in the order Tick() fired
    std::vector<int> render_order;  // ids in the order Render() painted

    // Live count doubles as stack depth: only TestStates touch the tracker, so
    // (constructed - destroyed) is exactly how many are currently on the stack.
    [[nodiscard]] int Live() const { return constructed - destroyed; }
  };

  // Test double for states::State. Construction/destruction bump the tracker; Tick()
  // hands back a scriptable request so we can drive StateFSM::Tick()'s dispatch through
  // every RequestType; renders_below is settable to exercise the visibility walk; Tick()
  // and Render() append the id so call/paint order is observable.
  struct TestState : states::State {
    Tracker* tracker;
    int id;
    bool renders_below;
    std::optional<states::StateRequest> scripted;

    TestState(Tracker* t, int state_id, bool render_below = false)
        : tracker(t), id(state_id), renders_below(render_below) {
      tracker->constructed++;
    }
    TestState(const TestState& other) = delete;
    TestState(TestState&& other) = delete;
    TestState& operator=(const TestState& other) = delete;
    TestState& operator=(TestState&& other) = delete;
    ~TestState() override { tracker->destroyed++; }

    std::optional<states::StateRequest> Tick() override {
      tracker->tick_order.push_back(id);
      return std::exchange(scripted, std::nullopt);
    }
    void Render() override { tracker->render_order.push_back(id); }
    [[nodiscard]] bool DoesRenderBelow() const override { return renders_below; }
  };

  // --- GetRenderRange: the layer-selection algorithm, extracted out of Render() ---

  TEST_CASE("GetRenderRange returns the lowest visible index") {
    Tracker tracker;
    std::vector<std::unique_ptr<states::State>> stack;

    SUBCASE("single element -> 0") {
      stack.push_back(std::make_unique<TestState>(&tracker, 0, false));
      CHECK(core::GetRenderRange(stack) == 0U);
    }
    SUBCASE("opaque top -> the top index") {
      stack.push_back(std::make_unique<TestState>(&tracker, 0, false));
      stack.push_back(std::make_unique<TestState>(&tracker, 1, false));
      CHECK(core::GetRenderRange(stack) == 1U);
    }
    SUBCASE("top renders below one opaque state -> the state beneath") {
      stack.push_back(std::make_unique<TestState>(&tracker, 0, false));
      stack.push_back(std::make_unique<TestState>(&tracker, 1, true));
      CHECK(core::GetRenderRange(stack) == 0U);
    }
    SUBCASE("an unbroken chain of render-below -> the bottom (0)") {
      stack.push_back(std::make_unique<TestState>(&tracker, 0, true));
      stack.push_back(std::make_unique<TestState>(&tracker, 1, true));
      stack.push_back(std::make_unique<TestState>(&tracker, 2, true));
      CHECK(core::GetRenderRange(stack) == 0U);
    }
  }

  // --- Push / Pop: the live count moves with the stack ---

  TEST_CASE("StateFSM::Push and Pop move the live state count") {
    Tracker tracker;
    core::StateFSM fsm;  // boots with one real MainMenu beneath the doubles

    CHECK(tracker.Live() == 0);

    fsm.Push(std::make_unique<TestState>(&tracker, 1));
    CHECK(tracker.Live() == 1);

    fsm.Push(std::make_unique<TestState>(&tracker, 2));
    CHECK(tracker.Live() == 2);

    fsm.Pop();
    CHECK(tracker.Live() == 1);

    fsm.Pop();
    CHECK(tracker.Live() == 0);
    CHECK_FALSE(fsm.Empty());  // the MainMenu the FSM booted with is still there
  }

  TEST_CASE("StateFSM::Pop on the last state empties the stack") {
    core::StateFSM fsm;  // single MainMenu
    REQUIRE_FALSE(fsm.Empty());

    fsm.Pop();  // logs a warning as a side effect (not observable here); stack drains
    CHECK(fsm.Empty());
  }

  // --- Tick dispatch: one case per RequestType. This is the heart of the suite, the part
  //     that must keep holding if TickTop is ever refactored. ---

  TEST_CASE("StateFSM::Tick dispatch: kPush grows the stack and puts the new state on top") {
    Tracker tracker;
    core::StateFSM fsm;

    auto top = std::make_unique<TestState>(&tracker, 1);
    top->scripted = states::StateRequest{.type = states::RequestType::kPush,
                                         .payload = std::make_unique<TestState>(&tracker, 2)};
    fsm.Push(std::move(top));
    REQUIRE(tracker.Live() == 2);  // state 1 live, state 2 already built as the payload

    fsm.Tick();  // ticks state 1 -> kPush(state 2)
    CHECK(tracker.Live() == 2);

    // State 2 is now the top: a render with no render-below flags paints only the top.
    fsm.Render();
    CHECK(tracker.render_order == std::vector<int>{2});
  }

  TEST_CASE("StateFSM::Tick dispatch: kPop removes the top state") {
    Tracker tracker;
    core::StateFSM fsm;

    auto top = std::make_unique<TestState>(&tracker, 1);
    top->scripted = states::StateRequest{.type = states::RequestType::kPop, .payload = nullptr};
    fsm.Push(std::move(top));
    REQUIRE(tracker.Live() == 1);

    fsm.Tick();  // ticks state 1 -> kPop
    CHECK(tracker.Live() == 0);
    CHECK_FALSE(fsm.Empty());  // MainMenu remains
  }

  TEST_CASE("StateFSM::Tick dispatch: kReplace destroys the old top and builds the new one") {
    Tracker tracker;
    core::StateFSM fsm;

    auto top = std::make_unique<TestState>(&tracker, 1);
    top->scripted = states::StateRequest{.type = states::RequestType::kReplace,
                                         .payload = std::make_unique<TestState>(&tracker, 2)};
    fsm.Push(std::move(top));
    REQUIRE(tracker.constructed == 2);
    REQUIRE(tracker.Live() == 2);

    fsm.Tick();  // ticks state 1 -> kReplace(state 2): state 1 destroyed, state 2 takes its slot
    CHECK(tracker.destroyed == 1);  // exactly the old top
    CHECK(tracker.Live() == 1);     // count unchanged: 2 replaced 1

    fsm.Render();
    CHECK(tracker.render_order == std::vector<int>{2});  // state 2 is the new top
  }

  TEST_CASE("StateFSM::Tick dispatch: kQuit clears the stack and destroys every state") {
    Tracker tracker;
    core::StateFSM fsm;

    fsm.Push(std::make_unique<TestState>(&tracker, 1));
    auto top = std::make_unique<TestState>(&tracker, 2);
    top->scripted = states::StateRequest{.type = states::RequestType::kQuit, .payload = nullptr};
    fsm.Push(std::move(top));
    REQUIRE(tracker.Live() == 2);

    fsm.Tick();  // ticks state 2 -> kQuit
    CHECK(fsm.Empty());
    CHECK(tracker.Live() == 0);  // both doubles (and the MainMenu) destroyed
  }

  TEST_CASE("StateFSM::Tick dispatch: a nullopt request leaves the stack unchanged") {
    Tracker tracker;
    core::StateFSM fsm;

    fsm.Push(std::make_unique<TestState>(&tracker, 1));  // empty scripted -> Tick returns nullopt
    fsm.Tick();
    CHECK(tracker.Live() == 1);
    CHECK(tracker.constructed == 1);
    CHECK(tracker.destroyed == 0);
  }

  // --- Invariants TECH.md locks in: tick the top only, paint bottom-up ---

  TEST_CASE("StateFSM::Tick only ticks the top state") {
    Tracker tracker;
    core::StateFSM fsm;

    fsm.Push(std::make_unique<TestState>(&tracker, 1));  // beneath
    fsm.Push(std::make_unique<TestState>(&tracker, 2));  // top
    fsm.Tick();

    CHECK(tracker.tick_order == std::vector<int>{2});  // only the top fired
  }

  TEST_CASE("StateFSM::Render paints the visible range bottom-up") {
    Tracker tracker;
    core::StateFSM fsm;

    // [MainMenu, state 1 (opaque), state 2 (renders below)].
    fsm.Push(std::make_unique<TestState>(&tracker, 1, /*render_below=*/false));
    fsm.Push(std::make_unique<TestState>(&tracker, 2, /*render_below=*/true));
    fsm.Render();

    // Top (2) renders below -> include state 1; state 1 is opaque -> stop above MainMenu.
    // Paint order across that visible range is bottom-up: 1 then 2.
    CHECK(tracker.render_order == std::vector<int>{1, 2});
  }

}  // namespace
