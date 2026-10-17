#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string_view>

namespace serein {

struct ProcessLaunchResult {
    bool started{false};
    bool timed_out{false};
    std::uint32_t exit_code{0U};
    std::uint32_t error_code{0U};
};

[[nodiscard]] ProcessLaunchResult launch_process_and_wait(
    const std::filesystem::path& executable,
    std::wstring_view arguments,
    std::chrono::milliseconds timeout);

}  // namespace serein
