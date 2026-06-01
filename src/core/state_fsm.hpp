#pragma once

#include <memory>
#include <vector>

#include "states.hpp"

class StateFSM {
 private:
  std::vector<std::unique_ptr<states::State>> _stack;

 public:
  StateFSM() = default;
  StateFSM(StateFSM& other) = delete;
  StateFSM(StateFSM&& other) = delete;
  void operator=(StateFSM& other) = delete;
  void operator=(StateFSM&& other) = delete;
  ~StateFSM() = default;

  void TickTop();
  void Render();

  void Push(std::unique_ptr<states::State> state_ptr);
  void Pop();
};
