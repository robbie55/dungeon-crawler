#include "state_fsm.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <utility>

#include "fsm_helpers.hpp"
#include "logger.hpp"
#include "states.hpp"

namespace core {

  void StateFSM::Push(std::unique_ptr<states::State> state_ptr) {
    _stack.emplace_back(std::move(state_ptr));
  }

  void StateFSM::Pop() {
    if (_stack.size() == 1) {
      LOG_WARN("StateFSM::POP() -> Called pop on last item in stack, will result in a game exit");
    }
    _stack.pop_back();
  }

  void StateFSM::Replace(std::unique_ptr<states::State> state_ptr) {
    _stack.back() = std::move(state_ptr);
  }

  void StateFSM::Tick() {
    assert(std::size(_stack) > 0 &&
           "[[static_assert]] StateFSM::TickTop() -> stack is attempting to Tick with no "
           "items in stack");

    std::optional<states::StateRequest> state_req{_stack.back()->Tick()};

    if (!state_req) {
      return;
    }

    switch (state_req->type) {
      case states::RequestType::kPush: {
        assert(state_req->payload && "StateFSM::TickTop() -> kPush operation missing payload");
        this->Push(std::move(state_req->payload));
        break;
      }
      case states::RequestType::kPop: {
        assert(!state_req->payload && "StateFSM::TickTop() -> kPop operation contains payload");
        this->Pop();
        break;
      }
      case states::RequestType::kReplace: {
        assert(state_req->payload && "StateFSM::TickTop() -> kReplace operation missing payload");
        this->Replace(std::move(state_req->payload));
        break;
      }
      case states::RequestType::kQuit: {
        _stack.clear();
        break;
      }
    }
  }

  void StateFSM::Render() {
    std::size_t render_at{GetRenderRange(_stack)};
    for (std::size_t i{render_at}; i < _stack.size(); ++i) {
      _stack[i]->Render();
    }
  }
}  // namespace core
