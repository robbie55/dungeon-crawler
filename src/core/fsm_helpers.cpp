#include "fsm_helpers.hpp"

namespace core {
  std::size_t GetRenderRange(const std::vector<std::unique_ptr<states::State>>& stack) {
    std::size_t render_at{};

    // this loop utilizes the fact a size_t will wrap around, will iterate to 0, then to a max...
    // size_t, which is greater than stack size, this is to prevent having to work around an int...
    // ...iterator and a therefore narrowing conversion into render_at
    for (std::size_t i{stack.size() - 1}; i < stack.size(); --i) {
      render_at = i;
      if (!stack[i]->DoesRenderBelow()) {
        break;
      }
    }

    return render_at;
  }
}  // namespace core
