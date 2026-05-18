#include <doctest/doctest.h>

#include "dungeon/grid/tile_coord.h"

namespace dungeon::grid {

  TEST_CASE("TileCoord equality") {
    CHECK(TileCoord{0, 0} == TileCoord{0, 0});
    CHECK(TileCoord{3, -4} == TileCoord{3, -4});
    CHECK_FALSE(TileCoord{1, 2} == TileCoord{2, 1});
  }

  TEST_CASE("TileCoord Manhattan distance") {
    CHECK(Manhattan({0, 0}, {0, 0}) == 0);
    CHECK(Manhattan({0, 0}, {3, 4}) == 7);
    CHECK(Manhattan({-2, -3}, {2, 3}) == 10);
    CHECK(Manhattan({5, 5}, {5, 5}) == 0);
  }

}  // namespace dungeon::grid
