#include "platform/Win32Application.h"
#include "platform/ProcessLauncher.h"
#include "platform/WindowsCredentialStore.h"
#include "render/D3D11Renderer.h"
#include "render/TextureStore.h"

#include <array>
#include <chrono>
#include <cstdlib>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

namespace {

bool test_dpapi_credential_store_round_trip() {
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / (L"serein-credentials-test-" + std::to_wstring(GetCurrentProcessId()));
    const std::filesystem::path file = directory / L"credentials.dat";
    std::error_code error;
    std::filesystem::remove_all(directory, error);

    serein::WindowsCredentialStore store{file};
    if (!store.save({"remembered.user", "super-secret-pass"})) {
        std::cerr << "DPAPI credential save failed.\n";
        return false;
    }
    const auto loaded = store.load();
    if (!loaded.has_value() || loaded->username != "remembered.user" || loaded->password != "super-secret-pass") {
        std::cerr << "DPAPI credential round-trip failed.\n";
        return false;
    }

    std::ifstream stream{file, std::ios::binary};
    const std::string bytes{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
    stream.close();
    if (bytes.find("remembered.user") != std::string::npos || bytes.find("super-secret-pass") != std::string::npos) {
        std::cerr << "Credential file contains plaintext input.\n";
        return false;
    }
    const bool cleared = store.clear();
    error.clear();
    const bool file_still_exists = std::filesystem::exists(file, error);
    if (!cleared || file_still_exists || error) {
        std::cerr << "Credential clear failed: cleared=" << cleared
                  << ", exists=" << file_still_exists
                  << ", path=" << file.string()
                  << ", filesystem=" << error.message() << '\n';
        return false;
    }
    std::filesystem::remove_all(directory, error);
    return true;
}

bool test_native_process_launch_smoke() {
    std::array<wchar_t, 32768> executable_path{};
    const DWORD length = GetModuleFileNameW(nullptr, executable_path.data(), static_cast<DWORD>(executable_path.size()));
    if (length == 0U || length >= executable_path.size()) {
        std::cerr << "Could not resolve the process smoke-test executable.\n";
        return false;
    }
    const serein::ProcessLaunchResult result = serein::launch_process_and_wait(
        std::filesystem::path{executable_path.data()}, L"--process-smoke-child", std::chrono::seconds{5});
    if (!result.started || result.timed_out || result.exit_code != 17U) {
        std::cerr << "Native EXE launch smoke test failed.\n";
        return false;
    }
    return true;
}

}  // namespace

int main(const int argument_count, char** arguments) {
    if (argument_count > 1 && std::string_view{arguments[1]} == "--process-smoke-child") {
        return 17;
    }
    static_assert(serein::Win32Application::client_width == 806);
    static_assert(serein::Win32Application::client_height == 468);
    static_assert(!std::is_copy_constructible_v<serein::D3D11Renderer>);
    static_assert(!std::is_copy_assignable_v<serein::D3D11Renderer>);

    if (!serein::TextureStore::supports_extension(L".jpg") ||
        !serein::TextureStore::supports_extension(L".jpeg") ||
        !serein::TextureStore::supports_extension(L".png") ||
        serein::TextureStore::supports_extension(L".webp")) {
        std::cerr << "Texture extension contract failed.\n";
        return EXIT_FAILURE;
    }
    if (!test_dpapi_credential_store_round_trip()) {
        return EXIT_FAILURE;
    }
    if (!test_native_process_launch_smoke()) {
        return EXIT_FAILURE;
    }

    const std::vector<std::byte> pixels{
        std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255},
        std::byte{0}, std::byte{0}, std::byte{0}, std::byte{255},
        std::byte{0}, std::byte{0}, std::byte{255}, std::byte{255},
    };
    const auto blurred = serein::blur_rgba_pixels(pixels, 3U, 1U, 1U);
    if (blurred.size() != pixels.size() ||
        std::to_integer<unsigned int>(blurred[4]) == 0U ||
        std::to_integer<unsigned int>(blurred[6]) == 0U ||
        std::to_integer<unsigned int>(blurred[7]) != 255U) {
        std::cerr << "Blurred companion texture contract failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "Serein platform contracts passed.\n";
    return EXIT_SUCCESS;
}
