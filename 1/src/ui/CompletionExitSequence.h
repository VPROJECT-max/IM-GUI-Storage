#pragma once

#include "integration/Services.h"
#include "ui/Animation.h"

#include <algorithm>

namespace serein::ui {

class CompletionExitSequence {
public:
    static constexpr float terminal_hold_seconds = 0.75F;
    static constexpr float fade_seconds = 0.65F;

    void update(const OperationState state, const float delta_seconds) noexcept {
        if (state != OperationState::succeeded && state != OperationState::failed) {
            elapsed_seconds_ = 0.0F;
            opacity_ = 1.0F;
            close_requested_ = false;
            return;
        }

        elapsed_seconds_ += std::max(delta_seconds, 0.0F);
        const float fade_progress = animation::clamp01(
            (elapsed_seconds_ - terminal_hold_seconds) / fade_seconds);
        opacity_ = 1.0F - animation::ease_in_out_cubic(fade_progress);
        close_requested_ = fade_progress >= 1.0F;
    }

    [[nodiscard]] float opacity() const noexcept {
        return opacity_;
    }

    [[nodiscard]] bool close_requested() const noexcept {
        return close_requested_;
    }

private:
    float elapsed_seconds_{0.0F};
    float opacity_{1.0F};
    bool close_requested_{false};
};

}  // namespace serein::ui
