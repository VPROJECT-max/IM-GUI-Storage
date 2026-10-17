#include "platform/WindowsCredentialStore.h"

#include <windows.h>
#include <dpapi.h>
#include <shlobj.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <utility>
#include <vector>

namespace serein {
namespace {

constexpr std::array<unsigned char, 4> file_magic{'S', 'R', 'N', '1'};
constexpr std::size_t maximum_field_size = 4096U;
constexpr std::size_t maximum_file_size = 1024U * 1024U;

void erase_bytes(std::vector<unsigned char>& bytes) noexcept {
    if (!bytes.empty()) {
        SecureZeroMemory(bytes.data(), bytes.size());
    }
}

std::filesystem::path temporary_path_for(const std::filesystem::path& path) {
    std::filesystem::path temporary = path;
    temporary += L".tmp";
    return temporary;
}

}  // namespace

std::filesystem::path default_credential_file_path() {
    PWSTR local_app_data = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &local_app_data)) ||
        local_app_data == nullptr) {
        return {};
    }
    const std::filesystem::path result = std::filesystem::path{local_app_data} / L"Serein" / L"credentials.dat";
    CoTaskMemFree(local_app_data);
    return result;
}

WindowsCredentialStore::WindowsCredentialStore(std::filesystem::path file_path)
    : file_path_(std::move(file_path)) {}

std::optional<RememberedCredentials> WindowsCredentialStore::load() {
    if (file_path_.empty()) {
        return std::nullopt;
    }
    std::ifstream stream{file_path_, std::ios::binary};
    if (!stream) {
        return std::nullopt;
    }
    std::vector<unsigned char> bytes{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
    if (bytes.size() <= file_magic.size() || bytes.size() > maximum_file_size ||
        !std::equal(file_magic.begin(), file_magic.end(), bytes.begin())) {
        return std::nullopt;
    }

    DATA_BLOB encrypted{};
    encrypted.cbData = static_cast<DWORD>(bytes.size() - file_magic.size());
    encrypted.pbData = bytes.data() + file_magic.size();
    DATA_BLOB decrypted{};
    LPWSTR description = nullptr;
    if (CryptUnprotectData(&encrypted, &description, nullptr, nullptr, nullptr,
                           CRYPTPROTECT_UI_FORBIDDEN, &decrypted) == FALSE) {
        return std::nullopt;
    }
    if (description != nullptr) {
        LocalFree(description);
    }

    const unsigned char* separator = std::find(decrypted.pbData, decrypted.pbData + decrypted.cbData, 0U);
    std::optional<RememberedCredentials> result;
    if (separator != decrypted.pbData + decrypted.cbData) {
        const std::size_t username_size = static_cast<std::size_t>(separator - decrypted.pbData);
        const std::size_t password_size = decrypted.cbData - username_size - 1U;
        if (username_size > 0U && username_size <= maximum_field_size &&
            password_size > 0U && password_size <= maximum_field_size) {
            result = RememberedCredentials{
                std::string{reinterpret_cast<const char*>(decrypted.pbData), username_size},
                std::string{reinterpret_cast<const char*>(separator + 1), password_size}};
        }
    }
    SecureZeroMemory(decrypted.pbData, decrypted.cbData);
    LocalFree(decrypted.pbData);
    return result;
}

bool WindowsCredentialStore::save(const RememberedCredentials& credentials) {
    if (file_path_.empty() || credentials.username.empty() || credentials.password.empty() ||
        credentials.username.size() > maximum_field_size || credentials.password.size() > maximum_field_size ||
        credentials.username.find('\0') != std::string::npos || credentials.password.find('\0') != std::string::npos) {
        return false;
    }

    std::vector<unsigned char> plaintext;
    plaintext.reserve(credentials.username.size() + credentials.password.size() + 1U);
    plaintext.insert(plaintext.end(), credentials.username.begin(), credentials.username.end());
    plaintext.push_back(0U);
    plaintext.insert(plaintext.end(), credentials.password.begin(), credentials.password.end());
    DATA_BLOB input{static_cast<DWORD>(plaintext.size()), plaintext.data()};
    DATA_BLOB encrypted{};
    const BOOL protected_ok = CryptProtectData(&input, L"Serein remembered credentials", nullptr, nullptr, nullptr,
                                                CRYPTPROTECT_UI_FORBIDDEN, &encrypted);
    erase_bytes(plaintext);
    if (protected_ok == FALSE) {
        return false;
    }

    std::error_code error;
    std::filesystem::create_directories(file_path_.parent_path(), error);
    const std::filesystem::path temporary = temporary_path_for(file_path_);
    bool written = false;
    if (!error) {
        std::ofstream stream{temporary, std::ios::binary | std::ios::trunc};
        if (stream) {
            stream.write(reinterpret_cast<const char*>(file_magic.data()), static_cast<std::streamsize>(file_magic.size()));
            stream.write(reinterpret_cast<const char*>(encrypted.pbData), static_cast<std::streamsize>(encrypted.cbData));
            stream.flush();
            written = stream.good();
        }
    }
    LocalFree(encrypted.pbData);

    if (!written || MoveFileExW(temporary.c_str(), file_path_.c_str(),
                                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == FALSE) {
        std::filesystem::remove(temporary, error);
        return false;
    }
    return true;
}

bool WindowsCredentialStore::clear() {
    if (file_path_.empty()) {
        return true;
    }
    std::error_code error;
    std::filesystem::remove(file_path_, error);
    if (error) {
        return false;
    }
    std::filesystem::remove(temporary_path_for(file_path_), error);
    return !error;
}

}  // namespace serein
