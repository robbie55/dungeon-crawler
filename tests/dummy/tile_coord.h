#pragma once

namespace dummy {

  struct TileCoord {
    int x;
    int y;
  };

  constexpr bool operator==(TileCoord a, TileCoord b) { return a.x == b.x && a.y == b.y; }

  int Manhattan(TileCoord a, TileCoord b);

}  // namespace dummy
