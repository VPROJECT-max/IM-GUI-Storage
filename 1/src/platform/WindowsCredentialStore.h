#pragma once

#include "integration/Services.h"

#include <filesystem>

namespace serein {

[[nodiscard]] std::filesystem::path default_credential_file_path();

class WindowsCredentialStore final : public ICredentialStore {
public:
    explicit WindowsCredentialStore(std::filesystem::path file_path = default_credential_file_path());

    [[nodiscard]] std::optional<RememberedCredentials> load() override;
    bool save(const RememberedCredentials& credentials) override;
    bool clear() override;

private:
    std::filesystem::path file_path_;
};

}  // namespace serein
