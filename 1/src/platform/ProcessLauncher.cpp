#include "platform/ProcessLauncher.h"

#include <windows.h>

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

namespace serein {

ProcessLaunchResult launch_process_and_wait(const std::filesystem::path& executable,
                                            const std::wstring_view arguments,
                                            const std::chrono::milliseconds timeout) {
    ProcessLaunchResult result;
    if (executable.empty()) {
        result.error_code = ERROR_INVALID_PARAMETER;
        return result;
    }

    std::wstring command_line = L"\"" + executable.wstring() + L"\"";
    if (!arguments.empty()) {
        command_line += L" ";
        command_line.append(arguments);
    }
    std::vector<wchar_t> mutable_command{command_line.begin(), command_line.end()};
    mutable_command.push_back(L'\0');

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (CreateProcessW(executable.c_str(), mutable_command.data(), nullptr, nullptr, FALSE,
                       CREATE_NO_WINDOW, nullptr, executable.parent_path().c_str(), &startup, &process) == FALSE) {
        result.error_code = GetLastError();
        return result;
    }
    result.started = true;
    CloseHandle(process.hThread);

    const auto clamped_timeout = std::clamp<std::int64_t>(
        timeout.count(), 0LL, static_cast<std::int64_t>((std::numeric_limits<DWORD>::max)()) - 1LL);
    const DWORD wait_result = WaitForSingleObject(process.hProcess, static_cast<DWORD>(clamped_timeout));
    if (wait_result == WAIT_TIMEOUT) {
        result.timed_out = true;
        result.error_code = WAIT_TIMEOUT;
        TerminateProcess(process.hProcess, ERROR_TIMEOUT);
        WaitForSingleObject(process.hProcess, 1000U);
    } else if (wait_result == WAIT_OBJECT_0) {
        DWORD exit_code = 0U;
        if (GetExitCodeProcess(process.hProcess, &exit_code) != FALSE) {
            result.exit_code = exit_code;
        } else {
            result.error_code = GetLastError();
        }
    } else {
        result.error_code = GetLastError();
    }
    CloseHandle(process.hProcess);
    return result;
}

}  // namespace serein
