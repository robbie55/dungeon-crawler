#include "dungeon/grid/tile_coord.h"

#include <cstdlib>

namespace dungeon::grid {

  int Manhattan(TileCoord a, TileCoord b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
  }

}  // namespace dungeon::grid
