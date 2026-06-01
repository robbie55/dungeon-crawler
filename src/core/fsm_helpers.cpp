#include "fsm_helpers.hpp"

namespace core {
  std::size_t GetRenderRange(const std::vector<std::unique_ptr<states::State>>& stack) {
    std::size_t render_at{};
    for (std::size_t i{stack.size() - 1}; i < stack.size(); --i) {
      render_at = i;
      if (!stack[i]->DoesRenderBelow()) {
        break;
      }
    }

    return render_at;
  }
}  // namespace core
