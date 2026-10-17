#include "platform/Win32Application.h"

#include <objbase.h>
#include <windows.h>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show_command) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    const HRESULT com_result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    serein::Win32Application application{instance};
    const int result = application.run(show_command);
    if (SUCCEEDED(com_result)) {
        CoUninitialize();
    }
    return result;
}
