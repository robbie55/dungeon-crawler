#include <raylib.h>

#include "core/logger.hpp"
#include "core/state_fsm.hpp"

namespace {
  void RenderFrame(const RenderTexture2D target, const Rectangle source_rec,
                   const Rectangle dest_rec, core::StateFSM& state_fsm) {
    // render game into virtual texture
    BeginTextureMode(target);
    {
      ClearBackground(RAYWHITE);

      // draw game obj's here
      state_fsm.Render();
    }
    EndTextureMode();

    BeginDrawing();
    {
      ClearBackground(BLACK);
      DrawTexturePro(target.texture, source_rec, dest_rec, {.x = 0.0F, .y = 0.0F}, 0.0F, WHITE);
    }
    EndDrawing();
  }
}  // namespace

int main() {
  core::log::Init();
  LOG_INFO("dungeon-crawler starting up");

  // virtual res values
  constexpr int kGameWidth{384};
  constexpr int kGameHeight{216};

  // these are the window sizes
  int screen_width{kGameWidth};
  int screen_height{kGameHeight};

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);

  InitWindow(screen_width, screen_height, "raylib - basic window");
  ToggleBorderlessWindowed();

  const RenderTexture2D kTarget{LoadRenderTexture(kGameWidth, kGameHeight)};

  const Rectangle kSourceRec{.x = 0.0F,
                             .y = 0.0F,
                             .width = static_cast<float>(kTarget.texture.width),
                             .height = static_cast<float>(-kTarget.texture.height)};

  // TODO: scales up fine for 16:9, need to make a decision on how we want it to behave for
  // ultrawide 16:10 either will create a distorted look, or will need to be accounted for
  Rectangle dest_rec{.x = 0.0F,
                     .y = 0.0F,
                     .width = static_cast<float>(screen_width),
                     .height = static_cast<float>(screen_height)};

  SetTargetFPS(60);

  core::StateFSM state_fsm{};

  while (!WindowShouldClose() && !state_fsm.Empty()) {
    if (IsWindowResized()) {
      screen_width = GetScreenWidth();
      screen_height = GetScreenHeight();
      dest_rec.width = static_cast<float>(screen_width);
      dest_rec.height = static_cast<float>(screen_height);
    }

    state_fsm.Tick();

    RenderFrame(kTarget, kSourceRec, dest_rec, state_fsm);
  }

  UnloadRenderTexture(kTarget);
  CloseWindow();

  LOG_INFO("dungeon-crawler shutting down");
  core::log::Shutdown();

  return 0;
}
