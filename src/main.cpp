#include <raylib.h>

#include "core/state_fsm.hpp"

namespace {
  void RenderFrame(const RenderTexture2D target, const Rectangle source_rec,
                   const Rectangle dest_rec) {
    // render game into virtual texture
    BeginTextureMode(target);
    {
      ClearBackground(RAYWHITE);
      DrawText("Scaled game!", 10, 10, 20, DARKGRAY);

      // draw game obj's here
    }
    EndTextureMode();

    BeginDrawing();
    {
      ClearBackground(BLACK);
      DrawTexturePro(target.texture, source_rec, dest_rec, {0.0F, 0.0F}, 0.0F, WHITE);
    }
    EndDrawing();
  }
}  // namespace

int main() {
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

  while (!WindowShouldClose()) {
    if (IsWindowResized()) {
      screen_width = GetScreenWidth();
      screen_height = GetScreenHeight();
      dest_rec.width = static_cast<float>(screen_width);
      dest_rec.height = static_cast<float>(screen_height);
    }

    RenderFrame(kTarget, kSourceRec, dest_rec);
  }

  UnloadRenderTexture(kTarget);
  CloseWindow();

  return 0;
}
