#include "ui/Animation.h"
#include "ui/CompletionExitSequence.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

int failures = 0;

void expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

float settle(const float frames_per_second) {
    float value = 0.0F;
    const float delta = 1.0F / frames_per_second;
    for (int frame = 0; frame < static_cast<int>(frames_per_second); ++frame) {
        value = serein::animation::damp(value, 1.0F, 8.0F, delta);
    }
    return value;
}

float advance_for(const float frames_per_second, const float seconds) {
    float value = 0.0F;
    const float delta = 1.0F / frames_per_second;
    for (float elapsed = 0.0F; elapsed < seconds; elapsed += delta) {
        value = serein::animation::advance_toward(value, 1.0F, 0.82F, delta);
    }
    return value;
}

}  // namespace

int main() {
    expect(serein::animation::clamp01(-3.0F) == 0.0F, "progress clamps below zero");
    expect(serein::animation::clamp01(4.0F) == 1.0F, "progress clamps above one");
    expect(serein::animation::ease_out_cubic(0.0F) == 0.0F, "ease starts at zero");
    expect(serein::animation::ease_out_cubic(1.0F) == 1.0F, "ease ends at one");

    float previous = -1.0F;
    for (int index = 0; index <= 100; ++index) {
        const float current = serein::animation::ease_in_out_cubic(static_cast<float>(index) / 100.0F);
        expect(current >= previous, "ease is monotonic");
        previous = current;
    }

    expect(std::abs(settle(60.0F) - settle(144.0F)) < 0.0005F, "damping is stable across refresh rates");
    expect(advance_for(60.0F, 0.40F) > 0.45F && advance_for(60.0F, 0.40F) < 0.55F,
           "timed motion is near halfway after 400ms");
    expect(advance_for(60.0F, 0.84F) == 1.0F, "timed motion reaches its target after the requested duration");
    expect(std::abs(advance_for(60.0F, 0.40F) - advance_for(144.0F, 0.40F)) < 0.02F,
           "timed motion is stable across refresh rates");

    serein::ui::CompletionExitSequence success;
    success.update(serein::OperationState::working, 5.0F);
    expect(success.opacity() == 1.0F && !success.close_requested(),
           "active loading never starts the exit sequence");
    success.update(serein::OperationState::succeeded, 0.70F);
    expect(success.opacity() == 1.0F && !success.close_requested(),
           "success remains fully readable during the terminal hold");
    success.update(serein::OperationState::succeeded, 0.35F);
    expect(success.opacity() < 1.0F && success.opacity() > 0.0F && !success.close_requested(),
           "success fades smoothly after the terminal hold");
    success.update(serein::OperationState::succeeded, 0.40F);
    expect(success.opacity() == 0.0F && success.close_requested(),
           "success requests close only after the fade finishes");

    serein::ui::CompletionExitSequence failure;
    failure.update(serein::OperationState::failed, 1.45F);
    expect(failure.opacity() == 0.0F && failure.close_requested(),
           "failure follows the same deliberate automatic-close sequence");
    failure.update(serein::OperationState::idle, 0.1F);
    expect(failure.opacity() == 1.0F && !failure.close_requested(),
           "idle state resets a completed exit sequence");

    if (failures == 0) {
        std::cout << "All Serein animation tests passed.\n";
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
