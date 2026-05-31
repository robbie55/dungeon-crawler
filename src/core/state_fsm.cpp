#include "state_fsm.hpp"

void StateFSM::Push(std::unique_ptr<states::State> state_ptr) {
  _stack.emplace_back(std::move(state_ptr));
}
void StateFSM::Pop() { _stack.pop_back(); };
