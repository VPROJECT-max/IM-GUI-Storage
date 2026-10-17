#include "app/AppModel.h"
#include "app/Configuration.h"
#include "integration/MockServices.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string_view>

namespace {

int failures = 0;

void expect(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void advance(serein::AppModel& model, const float seconds, const float step = 1.0F / 120.0F) {
    float elapsed = 0.0F;
    while (elapsed < seconds) {
        model.update(step);
        elapsed += step;
    }
}

void test_default_configuration_exposes_four_products_and_semver() {
    const auto config = serein::make_default_config();
    expect(config.version == "0.4.0", "final-polish release is version 0.4.0");
    expect(config.products.size() == 4U, "four products remain accessible");
    expect(config.products[0].name == "Rust", "Rust is the first product");
    expect(config.products[3].name == "Counter-Strike 2", "Counter-Strike 2 is the fourth product");
}

void test_empty_credentials_are_rejected_without_leaving_login() {
    auto auth = std::make_shared<serein::MockAuthenticator>();
    auto loader = std::make_shared<serein::MockProductLoader>();
    serein::AppModel model{serein::make_default_config(), auth, loader};

    model.set_credentials("", "");
    expect(!model.submit_login(), "empty credentials are rejected");
    expect(model.screen() == serein::Screen::authentication, "rejected login stays on authentication");
    expect(!model.login_error().empty(), "rejected login exposes recoverable feedback");
}

void test_demo_login_transitions_through_loading_to_products() {
    auto auth = std::make_shared<serein::MockAuthenticator>();
    auto loader = std::make_shared<serein::MockProductLoader>();
    serein::AppModel model{serein::make_default_config(), auth, loader};

    model.set_credentials("Taylor", "demo-pass");
    expect(model.submit_login(), "non-empty demo credentials authenticate");
    expect(model.screen() == serein::Screen::loading_products, "accepted login enters loading-products state");
    expect(model.current_user() == "Taylor", "authenticated username is retained");

    advance(model, 2.5F);
    expect(model.screen() == serein::Screen::loading_products, "product discovery remains visible long enough to read");
    advance(model, 0.7F);
    expect(model.screen() == serein::Screen::product_selection, "loading completes without blocking");
}

void test_selection_settings_and_username_share_one_model() {
    auto auth = std::make_shared<serein::MockAuthenticator>();
    auto loader = std::make_shared<serein::MockProductLoader>();
    serein::AppModel model{serein::make_default_config(), auth, loader};
    model.set_credentials("Mara", "pass");
    expect(model.submit_login(), "fixture login succeeds");
    advance(model, 3.2F);

    expect(model.select_product(2U), "valid product can be selected");
    expect(model.selected_product()->name == "Valorant", "selected metadata follows the card");
    expect(model.current_user() == "Mara", "details use the authenticated username");

    model.set_settings_open(true);
    expect(model.settings_open(), "settings can open");
    expect(model.load_state() == serein::OperationState::idle, "settings do not start product loading");

    expect(model.select_product(0U), "another product can replace current selection");
    expect(model.selected_product()->name == "Rust", "details update to new selection");
    model.close_selection();
    expect(model.selected_product() == nullptr, "closing details restores the full grid");
}

void test_product_load_is_idempotent_and_reports_success_or_failure() {
    auto auth = std::make_shared<serein::MockAuthenticator>();
    auto loader = std::make_shared<serein::MockProductLoader>();
    serein::AppModel model{serein::make_default_config(), auth, loader};
    model.set_credentials("Ari", "pass");
    expect(model.submit_login(), "fixture login succeeds");
    advance(model, 3.2F);
    expect(model.select_product(1U), "fixture product selection succeeds");

    expect(model.begin_product_load(), "load starts for a selected product");
    expect(!model.begin_product_load(), "repeated load clicks do not restart active work");
    expect(model.load_state() == serein::OperationState::working, "load reports working state");
    advance(model, 2.5F);
    expect(model.load_state() == serein::OperationState::working, "demo product loading remains visible past 2.5 seconds");
    advance(model, 0.7F);
    expect(model.load_state() == serein::OperationState::succeeded, "demo load completes successfully");
    expect(std::abs(model.load_progress() - 1.0F) < 0.001F, "successful progress reaches one");

    model.dismiss_load_result();
    loader->set_should_fail(true);
    expect(model.begin_product_load(), "load can run again after result dismissal");
    advance(model, 3.2F);
    expect(model.load_state() == serein::OperationState::failed, "configured demo failure is surfaced");
    expect(!model.load_message().empty(), "failure includes recovery copy");
    expect(model.load_message().find("Dismiss") == std::string_view::npos,
           "terminal failure copy does not ask for removed navigation");
}

void test_password_reveal_tracks_only_active_hold() {
    auto auth = std::make_shared<serein::MockAuthenticator>();
    auto loader = std::make_shared<serein::MockProductLoader>();
    serein::AppModel model{serein::make_default_config(), auth, loader};

    model.set_password_reveal_held(true);
    expect(model.password_reveal_held(), "password reveals while pointer is held");
    model.set_password_reveal_held(false);
    expect(!model.password_reveal_held(), "password hides immediately on release");
}

}  // namespace

int main() {
    test_default_configuration_exposes_four_products_and_semver();
    test_empty_credentials_are_rejected_without_leaving_login();
    test_demo_login_transitions_through_loading_to_products();
    test_selection_settings_and_username_share_one_model();
    test_product_load_is_idempotent_and_reports_success_or_failure();
    test_password_reveal_tracks_only_active_hold();

    if (failures == 0) {
        std::cout << "All Serein model tests passed.\n";
        return EXIT_SUCCESS;
    }
    std::cerr << failures << " Serein model test(s) failed.\n";
    return EXIT_FAILURE;
}
