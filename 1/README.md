# Serein — Loader Design 1

Serein is a premium Windows loader UI template built with C++20, DirectX 11, and Dear ImGui. Version `0.5.0` adds opt-in remembered login details, the final login artwork/brand alignment, and a tested native EXE-launch boundary while preserving the reference-matched loader flow.

This project contains no injection, game modification, credential collection, or remote authentication behavior. Its integrations are deliberately mock implementations intended to be replaced by authorized application logic.

## Build

Requirements:

- Windows 10 or newer
- Visual Studio 2022 or newer with Desktop development with C++
- CMake 3.25 or newer
- Network access on the first configure so CMake can fetch the pinned official Dear ImGui `v1.92.0` source

From the `1` directory:

```powershell
cmake -S . -B build -DSEREIN_BUILD_APP=ON -DSEREIN_BUILD_TESTS=ON
cmake --build build --config Debug
cmake --build build --config Release
ctest --test-dir build -C Debug --output-on-failure
ctest --test-dir build -C Release --output-on-failure
```

Run:

```powershell
.\build\Release\Serein.exe
```

The canonical executables are `build/Debug/Serein.exe` and `build/Release/Serein.exe`. The application uses the Windows subsystem, so no console window is created. Assets are copied beside both executables automatically.

## Demo flow

- The login is local demo behavior. Any non-empty username and password are accepted.
- The password is visible only while the eye control is actively held with the primary mouse button.
- When **Remember me** is enabled, a successful login restores the toggle, username, and password on the next launch. Turning it off deletes the saved entry immediately; failed logins never overwrite it.
- Product discovery is simulated for three seconds with a delta-time-driven state machine, so the loading screen stays responsive.
- Selecting a card opens the details panel with an 820 ms eased slide. Settings remains a local presentation overlay, and Load runs a three-second mock operation.
- After the mock operation reports success or failure, the result remains readable briefly, the window fades smoothly, and the application closes through its normal cleanup path without another button.

## Customization map

| Change | Location |
|---|---|
| Application name, version, accent seed, background, product names/status/dates/art | `src/app/Configuration.cpp` and `src/app/Configuration.h` |
| Product data shape | `src/app/Product.h` |
| Authentication integration | Implement `IAuthenticator` in `src/integration/Services.h`, then inject it in `src/platform/Win32Application.cpp` |
| Product launch/loading integration | Implement `IProductLoader` in `src/integration/Services.h`, then inject it in `src/platform/Win32Application.cpp` |
| Secure remembered-login storage | `src/platform/WindowsCredentialStore.cpp` |
| Native process-launch boundary | `src/platform/ProcessLauncher.cpp` |
| Demo behavior and duration | `src/integration/MockServices.cpp` |
| Colors, radii, spacing, ImGui style | `src/ui/Theme.h` and `src/ui/Theme.cpp` |
| Reusable controls and progress visuals | `src/ui/Components.cpp` |
| Login layout/copy | `src/ui/screens/AuthScreen.cpp` |
| Product cards and 2×2 grid layout | `src/ui/screens/ProductScreen.cpp` |
| Window size and native behavior | `src/platform/Win32Application.h` and `src/platform/Win32Application.cpp` |

Asset paths are centralized in `AppConfig`; rendering code never repeats product filenames.

## Remembered login storage

The opt-in credential file is `%LOCALAPPDATA%\Serein\credentials.dat`. Its contents are encrypted with Windows DPAPI using the current Windows user profile; the username and password are never written there as plaintext. The file is local to that Windows account and machine, and software already running as the same user may still be able to request DPAPI decryption. Clear the checkbox before signing in to remove the file.

## Integration notes

Keep network, authorization, and launching logic behind `IAuthenticator` and `IProductLoader`. The UI owns presentation only. A production loader should validate authorization on a trusted service, avoid retaining passwords, return clear failure messages, and keep long operations off the render thread.

`IProductLoader::update(float)` is polled once per frame. Implementations should report state/progress without blocking. If the real integration is asynchronous, synchronize a small status snapshot into this interface rather than waiting in the UI thread.

`launch_process_and_wait(...)` is a reusable Win32 EXE-launch boundary with timeout and handle cleanup. The platform contract suite proves it by starting a child copy of the test executable and verifying its exit code. It does not load DLLs, inject into another process, or connect the mock product cards to arbitrary executables.

## Review captures

Generate deterministic review captures with:

```powershell
.\build\Release\Serein.exe --capture-dir .\review
```

The app saves `login.png`, `products-loading.png`, `products.png`, `selected.png`, `product-loading.png`, and `product-success.png`, then exits automatically through the same completion path used by the interactive demo. The local `review` folder is generated verification output and is intentionally ignored by Git.

## Current limitations

- Authentication, product discovery, and product loading are mock behavior.
- Product discovery uses a frame-time-driven two-pass DirectX 11 Gaussian blur, which fades away into the selection grid.
- Dear ImGui provides keyboard navigation and clear focus states here, but it is not a complete native accessibility or complex-script text solution.
- The supplied artwork is project-owner material. Confirm redistribution rights before packaging the template.
