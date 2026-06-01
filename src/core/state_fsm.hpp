#pragma once

#include <memory>
#include <vector>

#include "states.hpp"

class StateFSM {
 private:
  std::vector<std::unique_ptr<states::State>> _stack;

 public:
  StateFSM() { _stack.emplace_back(std::make_unique<states::MainMenu>()); };
  StateFSM(const StateFSM& other) = delete;
  StateFSM(StateFSM&& other) = delete;
  StateFSM& operator=(const StateFSM& other) = delete;
  void operator=(StateFSM&& other) = delete;
  ~StateFSM() = default;

  void Tick();
  void Render();

  void Push(std::unique_ptr<states::State> state_ptr);
  void Pop();
};
