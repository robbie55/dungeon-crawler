#include "states.hpp"

std::optional<states::StateRequest> states::MainMenu::Tick() { return std::nullopt; }

std::optional<states::StateRequest> states::Hub::Tick() { return std::nullopt; }

std::optional<states::StateRequest> states::Level::Tick() { return std::nullopt; }

std::optional<states::StateRequest> states::GameOver::Tick() { return std::nullopt; }

std::optional<states::StateRequest> states::Victory::Tick() {
  frame_count++;
  if (frame_count <= kVictoryFrames) {
    return std::nullopt;
  }

  StateRequest req{.type = RequestType::kReplace, .payload = std::make_unique<Hub>()};

  return req;
}

std::optional<states::StateRequest> states::Pause::Tick() { return std::nullopt; }
