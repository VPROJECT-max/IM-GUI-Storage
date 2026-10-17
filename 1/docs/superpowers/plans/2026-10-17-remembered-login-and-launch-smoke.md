# Remembered Login and Launch Smoke Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Raise the Serein brand line, use the supplied login artwork, securely remember successful login inputs, prove native EXE launching, produce the canonical Debug/Release builds, and push the release.

**Architecture:** `AppModel` owns the remember-me state and uses an injected credential-store boundary. Production uses a Windows DPAPI store under LocalAppData; UI buffers initialize from the model. A small Windows process-launch helper is verified by launching the platform test executable as a child and checking its exit code.

**Tech Stack:** C++20, Dear ImGui, Win32, Windows DPAPI/Crypt32, CMake/CTest.

**Spec:** User request in the active Codex conversation dated 2026-10-17.

## Global Constraints

- Preserve the approved reference-matched composition except for the requested higher brand line and supplied artwork.
- Never persist credentials as plaintext; bind protected data to the current Windows user.
- Do not implement DLL injection or execute an arbitrary payload.
- Build `build/Debug/Serein.exe` and `build/Release/Serein.exe`.
- Commit only Design 1 changes and push `loader-designs` to `origin`.

## Review Focus

- Missing or malformed encrypted credential files must leave login usable with empty/default inputs.
- Failed authentication must not overwrite previously saved credentials.
- Turning Remember me off must remove saved credentials.
- Capture mode must not read or modify a developer's saved credentials.
- Process-launch timeout/failure must close handles and return a clear result.

---

### Task 1: Secure remembered login

**Files:**
- Create: `src/platform/WindowsCredentialStore.h`
- Create: `src/platform/WindowsCredentialStore.cpp`
- Modify: `src/integration/Services.h`
- Modify: `src/app/AppModel.h`
- Modify: `src/app/AppModel.cpp`
- Modify: `src/ui/screens/AuthScreen.h`
- Modify: `src/ui/screens/AuthScreen.cpp`
- Modify: `src/platform/Win32Application.cpp`
- Modify: `tests/AppModelTests.cpp`
- Modify: `tests/PlatformContractTests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces: `ICredentialStore`, `RememberedCredentials`, and `WindowsCredentialStore`.
- Consumes: authentication success and the existing login input model.

- [x] Write model tests for load-on-start, save-after-success, no-save-on-failure, and clear-on-uncheck.
- [x] Run the focused tests and verify the new cases fail before implementation.
- [x] Implement the credential-store boundary and AppModel remember-me behavior.
- [x] Add a DPAPI round-trip test that proves the on-disk bytes omit plaintext credentials.
- [x] Run focused tests until green.

### Task 2: Requested visual corrections and artwork

**Files:**
- Modify: `src/app/Configuration.h`
- Modify: `src/app/Configuration.cpp`
- Modify: `src/ui/LoaderUI.cpp`
- Modify: `src/ui/UiContracts.h`
- Modify: `src/ui/screens/AuthScreen.cpp`
- Modify: `tests/UiContractTests.cpp`
- Modify: `tests/AssetCaptureTests.cpp`

**Interfaces:**
- Consumes: model-backed remembered inputs from Task 1.
- Produces: raised 26 px Serein brand line and the configured Hazmat Warrior artwork.

- [x] Add failing UI/asset contract tests for the raised brand baseline and supplied artwork.
- [x] Implement only the requested baseline and asset-path changes.
- [x] Build and capture the login screen for native-scale visual review.
- [x] Run focused tests until green.

### Task 3: Native EXE launch smoke test and release

**Files:**
- Create: `src/platform/ProcessLauncher.h`
- Create: `src/platform/ProcessLauncher.cpp`
- Modify: `tests/PlatformContractTests.cpp`
- Modify: `CMakeLists.txt`
- Modify: `README.md`
- Modify: `src/app/Configuration.cpp`

**Interfaces:**
- Produces: `launch_process_and_wait(...) -> ProcessLaunchResult`.
- Consumes: the platform-test executable path for a benign child-process smoke run.

- [x] Add a failing platform smoke test that launches the test executable as a child and verifies exit code 17.
- [x] Implement timeout-safe Win32 process launching and make the test green.
- [x] Document `%LOCALAPPDATA%\\Serein\\credentials.dat`, DPAPI behavior, canonical build paths, and smoke-test scope.
- [x] Bump the feature release to `0.5.0`, build Debug and Release in `build/`, and run both complete suites.
- [x] Run ImGui static checks, recapture the requested login state, audit the diff, commit once, and push `loader-designs`.
