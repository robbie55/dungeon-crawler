#include "states.hpp"

using namespace states;

std::optional<StateRequest> MainMenu::Tick() {
  // if WorldEvent == player_pressed_play replace with hub
  // if WorldEvent == player_pressed_quit exit game

  return std::nullopt;

  StateRequest req{.type = RequestType::kReplace, .payload = std::make_unique<Hub>()};

  return req;
}

std::optional<StateRequest> Hub::Tick() {
  // if WorldEvent == player_entered_some_level
  // push some level
  return std::nullopt;
}

std::optional<StateRequest> Level::Tick() { return std::nullopt; }

std::optional<StateRequest> GameOver::Tick() {
  // if WorldEvent == player_presedcontinnue replace with hub
  // if WorldEvent == player_pressed_quit exit game

  return std::nullopt;

  StateRequest req{.type = RequestType::kReplace, .payload = std::make_unique<Hub>()};

  return req;
}

std::optional<StateRequest> Victory::Tick() {
  frame_count++;
  if (frame_count <= 15) {
    return std::nullopt;
  }

  StateRequest req{.type = RequestType::kReplace, .payload = std::make_unique<Hub>()};

  return req;
}

std::optional<StateRequest> Pause::Tick() {
  // if WorldEvent == player_pressed_pause
  // if (true) {
  return std::nullopt;
  // }

  StateRequest req{.type = RequestType::kReplace, .payload = std::make_unique<Hub>()};

  return req;
}
