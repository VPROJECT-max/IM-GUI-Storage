#pragma once

#include <algorithm>
#include <cmath>

namespace serein::animation {

[[nodiscard]] constexpr float clamp01(const float value) noexcept {
    return std::clamp(value, 0.0F, 1.0F);
}

[[nodiscard]] constexpr float ease_out_cubic(const float value) noexcept {
    const float t = clamp01(value);
    const float inverse = 1.0F - t;
    return 1.0F - inverse * inverse * inverse;
}

[[nodiscard]] constexpr float ease_in_out_cubic(const float value) noexcept {
    const float t = clamp01(value);
    if (t < 0.5F) {
        return 4.0F * t * t * t;
    }
    const float inverse = -2.0F * t + 2.0F;
    return 1.0F - (inverse * inverse * inverse) / 2.0F;
}

[[nodiscard]] inline float damp(const float current,
                                const float target,
                                const float response,
                                const float delta_seconds) noexcept {
    const float safe_delta = std::max(delta_seconds, 0.0F);
    const float amount = 1.0F - std::exp(-std::max(response, 0.0F) * safe_delta);
    return current + (target - current) * amount;
}

[[nodiscard]] constexpr float advance_toward(const float current,
                                             const float target,
                                             const float duration_seconds,
                                             const float delta_seconds) noexcept {
    if (duration_seconds <= 0.0F) {
        return target;
    }
    const float step = std::max(delta_seconds, 0.0F) / duration_seconds;
    if (current < target) {
        return std::min(current + step, target);
    }
    return std::max(current - step, target);
}

[[nodiscard]] inline float pulse(const float seconds, const float cycles_per_second = 1.0F) noexcept {
    constexpr float tau = 6.28318530718F;
    return 0.5F + 0.5F * std::sin(seconds * cycles_per_second * tau);
}

}  // namespace serein::animation
