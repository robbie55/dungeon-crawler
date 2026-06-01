#include "state_fsm.hpp"

#include <cassert>
#include <optional>
#include <utility>

#include "states.hpp"

void StateFSM::Push(std::unique_ptr<states::State> state_ptr) {
  _stack.emplace_back(std::move(state_ptr));
}

void StateFSM::Pop() { _stack.pop_back(); };

void StateFSM::TickTop() {
  std::optional<states::StateRequest> state_req{_stack[_stack.size() - 1]->Tick()};

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
      this->Pop();
      this->Push(std::move(state_req->payload));
      break;
    }
    case states::RequestType::kQuit: {
      break;
    }
  }
}

void StateFSM::Render() {
  std::size_t render_at{};
  for (int i{static_cast<int>(_stack.size()) - 1}; i >= 0; --i) {
    render_at = i;
    if (!_stack[i]->DoesRenderBelow()) {
      break;
    }
  }

  for (std::size_t i{render_at}; i < _stack.size(); ++i) {
    _stack[i]->Render();
  }
}
