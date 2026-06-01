#include "states.hpp"

using namespace states;

std::optional<StateRequest> MainMenu::Tick() { return std::nullopt; }

std::optional<StateRequest> Hub::Tick() { return std::nullopt; }

std::optional<StateRequest> Level::Tick() { return std::nullopt; }

std::optional<StateRequest> GameOver::Tick() { return std::nullopt; }

std::optional<StateRequest> Victory::Tick() {
  frame_count++;
  if (frame_count <= kVictoryFrames) {
    return std::nullopt;
  }

  StateRequest req{.type = RequestType::kReplace, .payload = std::make_unique<Hub>()};

  return req;
}

std::optional<StateRequest> Pause::Tick() { return std::nullopt; }
