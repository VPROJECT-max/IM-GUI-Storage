#include "integration/MockServices.h"

#include <algorithm>
#include <cctype>

namespace serein {
namespace {

bool contains_visible_character(const std::string_view value) {
    return std::ranges::any_of(value, [](const char character) {
        return std::isspace(static_cast<unsigned char>(character)) == 0;
    });
}

}  // namespace

bool MockAuthenticator::authenticate(const std::string_view username, const std::string_view password) {
    return contains_visible_character(username) && contains_visible_character(password);
}

MockProductLoader::MockProductLoader(const float duration_seconds)
    : duration_seconds_(std::max(duration_seconds, 0.1F)) {}

bool MockProductLoader::begin(const Product&) {
    if (state_ == OperationState::working) {
        return false;
    }
    elapsed_seconds_ = 0.0F;
    message_ = "Preparing a safe demo launch...";
    state_ = OperationState::working;
    return true;
}

void MockProductLoader::update(const float delta_seconds) {
    if (state_ != OperationState::working) {
        return;
    }
    elapsed_seconds_ += std::max(delta_seconds, 0.0F);
    if (elapsed_seconds_ < duration_seconds_) {
        return;
    }
    if (should_fail_) {
        state_ = OperationState::failed;
        message_ = "The demo load could not finish.";
        return;
    }
    state_ = OperationState::succeeded;
    message_ = "Product ready. Demo loading completed safely.";
}

void MockProductLoader::reset() {
    elapsed_seconds_ = 0.0F;
    state_ = OperationState::idle;
    message_.clear();
}

OperationState MockProductLoader::state() const noexcept {
    return state_;
}

float MockProductLoader::progress() const noexcept {
    if (state_ == OperationState::succeeded || state_ == OperationState::failed) {
        return 1.0F;
    }
    return std::clamp(elapsed_seconds_ / duration_seconds_, 0.0F, 1.0F);
}

std::string_view MockProductLoader::message() const noexcept {
    return message_;
}

void MockProductLoader::set_should_fail(const bool should_fail) noexcept {
    should_fail_ = should_fail;
}

}  // namespace serein
