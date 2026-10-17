#pragma once

#include "app/Configuration.h"
#include "integration/Services.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace serein {

enum class Screen {
    authentication,
    loading_products,
    product_selection,
};

class AppModel {
public:
    AppModel(AppConfig config,
             std::shared_ptr<IAuthenticator> authenticator,
             std::shared_ptr<IProductLoader> product_loader);

    void update(float delta_seconds);
    void set_credentials(std::string username, std::string password);
    [[nodiscard]] bool submit_login();
    [[nodiscard]] bool select_product(std::size_t index);
    void close_selection() noexcept;
    [[nodiscard]] bool begin_product_load();
    void dismiss_load_result();
    void set_settings_open(bool open) noexcept;
    void set_password_reveal_held(bool held) noexcept;

    [[nodiscard]] const AppConfig& config() const noexcept;
    [[nodiscard]] Screen screen() const noexcept;
    [[nodiscard]] std::string_view username_input() const noexcept;
    [[nodiscard]] std::string_view password_input() const noexcept;
    [[nodiscard]] std::string_view current_user() const noexcept;
    [[nodiscard]] std::string_view login_error() const noexcept;
    [[nodiscard]] const Product* selected_product() const noexcept;
    [[nodiscard]] std::optional<std::size_t> selected_product_index() const noexcept;
    [[nodiscard]] bool settings_open() const noexcept;
    [[nodiscard]] bool password_reveal_held() const noexcept;
    [[nodiscard]] OperationState load_state() const noexcept;
    [[nodiscard]] float load_progress() const noexcept;
    [[nodiscard]] std::string_view load_message() const noexcept;
    [[nodiscard]] float product_discovery_progress() const noexcept;

private:
    AppConfig config_;
    std::shared_ptr<IAuthenticator> authenticator_;
    std::shared_ptr<IProductLoader> product_loader_;
    Screen screen_{Screen::authentication};
    std::string username_input_;
    std::string password_input_;
    std::string current_user_;
    std::string login_error_;
    std::optional<std::size_t> selected_product_index_;
    float product_discovery_elapsed_{0.0F};
    bool settings_open_{false};
    bool password_reveal_held_{false};
};

}  // namespace serein
