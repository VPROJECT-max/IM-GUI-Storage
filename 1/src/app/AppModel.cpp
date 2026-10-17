#include "app/AppModel.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace serein {

AppModel::AppModel(AppConfig config,
                   std::shared_ptr<IAuthenticator> authenticator,
                   std::shared_ptr<IProductLoader> product_loader,
                   std::shared_ptr<ICredentialStore> credential_store)
    : config_(std::move(config)),
      authenticator_(std::move(authenticator)),
      product_loader_(std::move(product_loader)),
      credential_store_(std::move(credential_store)) {
    if (!authenticator_ || !product_loader_) {
        throw std::invalid_argument{"Serein services must not be null"};
    }
    if (credential_store_) {
        const std::optional<RememberedCredentials> remembered = credential_store_->load();
        if (remembered.has_value() && !remembered->username.empty() && !remembered->password.empty()) {
            username_input_ = remembered->username;
            password_input_ = remembered->password;
            remember_me_ = true;
        }
    }
}

void AppModel::update(const float delta_seconds) {
    const float safe_delta = std::max(delta_seconds, 0.0F);
    if (screen_ == Screen::loading_products) {
        product_discovery_elapsed_ += safe_delta;
        if (product_discovery_elapsed_ >= config_.product_discovery_seconds) {
            screen_ = Screen::product_selection;
        }
    }
    if (product_loader_->state() == OperationState::working) {
        product_loader_->update(safe_delta);
    }
}

void AppModel::set_credentials(std::string username, std::string password) {
    username_input_ = std::move(username);
    password_input_ = std::move(password);
    login_error_.clear();
}

bool AppModel::submit_login() {
    if (screen_ != Screen::authentication) {
        return false;
    }
    if (!authenticator_->authenticate(username_input_, password_input_)) {
        login_error_ = "Enter a username and password to continue.";
        return false;
    }
    if (remember_me_ && credential_store_) {
        static_cast<void>(credential_store_->save(RememberedCredentials{username_input_, password_input_}));
    }
    current_user_ = username_input_;
    password_input_.clear();
    login_error_.clear();
    product_discovery_elapsed_ = 0.0F;
    screen_ = Screen::loading_products;
    return true;
}

bool AppModel::select_product(const std::size_t index) {
    if (screen_ != Screen::product_selection || index >= config_.products.size()) {
        return false;
    }
    selected_product_index_ = index;
    settings_open_ = false;
    return true;
}

void AppModel::close_selection() noexcept {
    selected_product_index_.reset();
    settings_open_ = false;
}

bool AppModel::begin_product_load() {
    if (!selected_product_index_.has_value() || product_loader_->state() != OperationState::idle) {
        return false;
    }
    settings_open_ = false;
    return product_loader_->begin(config_.products[*selected_product_index_]);
}

void AppModel::dismiss_load_result() {
    if (product_loader_->state() != OperationState::working) {
        product_loader_->reset();
    }
}

void AppModel::set_settings_open(const bool open) noexcept {
    settings_open_ = open && selected_product_index_.has_value();
}

void AppModel::set_password_reveal_held(const bool held) noexcept {
    password_reveal_held_ = held;
}

void AppModel::set_remember_me(const bool remember) {
    remember_me_ = remember;
    if (!remember_me_ && credential_store_) {
        static_cast<void>(credential_store_->clear());
    }
}

const AppConfig& AppModel::config() const noexcept {
    return config_;
}

Screen AppModel::screen() const noexcept {
    return screen_;
}

std::string_view AppModel::username_input() const noexcept {
    return username_input_;
}

std::string_view AppModel::password_input() const noexcept {
    return password_input_;
}

std::string_view AppModel::current_user() const noexcept {
    return current_user_;
}

std::string_view AppModel::login_error() const noexcept {
    return login_error_;
}

const Product* AppModel::selected_product() const noexcept {
    if (!selected_product_index_.has_value()) {
        return nullptr;
    }
    return &config_.products[*selected_product_index_];
}

std::optional<std::size_t> AppModel::selected_product_index() const noexcept {
    return selected_product_index_;
}

bool AppModel::settings_open() const noexcept {
    return settings_open_;
}

bool AppModel::password_reveal_held() const noexcept {
    return password_reveal_held_;
}

bool AppModel::remember_me() const noexcept {
    return remember_me_;
}

OperationState AppModel::load_state() const noexcept {
    return product_loader_->state();
}

float AppModel::load_progress() const noexcept {
    return product_loader_->progress();
}

std::string_view AppModel::load_message() const noexcept {
    return product_loader_->message();
}

float AppModel::product_discovery_progress() const noexcept {
    return std::clamp(product_discovery_elapsed_ / config_.product_discovery_seconds, 0.0F, 1.0F);
}

}  // namespace serein
