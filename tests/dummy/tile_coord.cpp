#include "tile_coord.h"

#include <cstdlib>

namespace dummy {

  int Manhattan(TileCoord a, TileCoord b) { return std::abs(a.x - b.x) + std::abs(a.y - b.y); }

}  // namespace dummy
