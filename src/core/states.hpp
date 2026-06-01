#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

namespace states {
  struct StateRequest;

  // state abstract class, used to derive game states
  struct State {
    // TODO: This constructor handles allocating resources which each state has, likely but not
    // confirmed. Per state constructors may exist for larger states i.e. Level
    State() = default;

    State(const State& other) = delete;
    State(State&& other) = delete;
    State& operator=(const State& other) = delete;
    State& operator=(State&& other) = delete;
    virtual ~State() = default;

    virtual std::optional<StateRequest> Tick() = 0;
    virtual void Render() = 0;
    [[nodiscard]] virtual bool DoesRenderBelow() const { return false; };
  };

  enum class RequestType : std::uint8_t {
    kPush,
    kPop,
    kReplace,
    kQuit,
  };

  // State Ticks will return a StateRequest containing how said state will interact with state FSM
  struct StateRequest {
    RequestType type;
    std::unique_ptr<State> payload;
  };

  struct MainMenu : State {
    void Render() override {};
    std::optional<StateRequest> Tick() override;
  };
  struct Hub : State {
    void Render() override {};
    std::optional<StateRequest> Tick() override;
  };
  struct Level : State {
    // TODO: Player, enemy, projectile count, this truct WILL grow
    void Render() override {};
    std::optional<StateRequest> Tick() override;
  };
  struct Pause : State {
    void Render() override {};
    [[nodiscard]] bool DoesRenderBelow() const override { return true; };
    std::optional<StateRequest> Tick() override;
  };
  struct GameOver : State {
    // TBD: GameOver is an overlay of the curernt level, with fade in/gradient
    void Render() override {};
    [[nodiscard]] bool DoesRenderBelow() const override { return true; };
    std::optional<StateRequest> Tick() override;
  };
  struct Victory : State {
    std::size_t frame_count{};
    void Render() override {};
    std::optional<StateRequest> Tick() override;
  };

}  // namespace states
