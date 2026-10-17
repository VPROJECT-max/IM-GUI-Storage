#pragma once

#include "integration/Services.h"

#include <string>

namespace serein {

class MockAuthenticator final : public IAuthenticator {
public:
    [[nodiscard]] bool authenticate(std::string_view username, std::string_view password) override;
};

class MockProductLoader final : public IProductLoader {
public:
    explicit MockProductLoader(float duration_seconds = 3.0F);

    bool begin(const Product& product) override;
    void update(float delta_seconds) override;
    void reset() override;
    [[nodiscard]] OperationState state() const noexcept override;
    [[nodiscard]] float progress() const noexcept override;
    [[nodiscard]] std::string_view message() const noexcept override;

    void set_should_fail(bool should_fail) noexcept;

private:
    float duration_seconds_;
    float elapsed_seconds_{0.0F};
    bool should_fail_{false};
    OperationState state_{OperationState::idle};
    std::string message_;
};

}  // namespace serein
