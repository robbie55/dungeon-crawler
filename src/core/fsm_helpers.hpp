#pragma once

#include <cstddef>
#include <vector>

#include "states.hpp"

namespace core {
  std::size_t GetRenderRange(const std::vector<std::unique_ptr<states::State>>& stack);
}  // namespace core
