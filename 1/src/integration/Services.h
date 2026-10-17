#pragma once

#include "app/Product.h"

#include <string>
#include <string_view>
#include <optional>

namespace serein {

enum class OperationState {
    idle,
    working,
    succeeded,
    failed,
};

struct RememberedCredentials {
    std::string username;
    std::string password;
};

class ICredentialStore {
public:
    virtual ~ICredentialStore() = default;
    [[nodiscard]] virtual std::optional<RememberedCredentials> load() = 0;
    virtual bool save(const RememberedCredentials& credentials) = 0;
    virtual bool clear() = 0;
};

class IAuthenticator {
public:
    virtual ~IAuthenticator() = default;
    [[nodiscard]] virtual bool authenticate(std::string_view username, std::string_view password) = 0;
};

class IProductLoader {
public:
    virtual ~IProductLoader() = default;
    virtual bool begin(const Product& product) = 0;
    virtual void update(float delta_seconds) = 0;
    virtual void reset() = 0;
    [[nodiscard]] virtual OperationState state() const noexcept = 0;
    [[nodiscard]] virtual float progress() const noexcept = 0;
    [[nodiscard]] virtual std::string_view message() const noexcept = 0;
};

}  // namespace serein
