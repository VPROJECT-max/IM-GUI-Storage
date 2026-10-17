# Serein Final Visual Polish & Release Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Match the supplied reference typography, buttons, and dark surface treatment more closely, then replace post-load navigation with a brief terminal message, a smooth window fade, and clean automatic shutdown.

**Architecture:** Preserve the existing AppModel/service/UI separation and fixed 806×468 Win32 + DirectX 11 application. Put terminal hold/fade timing in a small testable UI sequence, keep the product overlay presentation-only, and let Win32Application own actual window opacity and shutdown.

**Tech Stack:** C++20, Dear ImGui v1.92.0, DirectX 11, Win32, CMake/CTest.

**Spec:** User-provided `Serein — Final Visual Polish & Release` brief dated 2026-10-17 and the six supplied original screenshots.

## Global Constraints

- Treat the supplied screenshots as the visual specification; do not add or redesign screens.
- Preserve Serein branding, supplied product artwork, current dimensions, demo-service boundary, and non-blocking rendering.
- Work only in Loader Design 1 on `loader-designs`; do not touch Loader Design 2 or `menu-designs`.
- Remove the post-load button and close automatically after a readable success or failure message.
- Build and verify Debug and Release, update SemVer to `0.4.0`, then commit and push `loader-designs`.
- Do not commit build products or generated review captures.

## Review Focus

- Success and failure both remain readable briefly, fade smoothly, request closure once, and reset while idle/working.
- The final opacity is applied to the real top-level window without bypassing the normal resource-cleanup path.
- Font fallback remains safe when Segoe UI files are unavailable.
- Shared button rendering keeps text and icons optically centered across primary and secondary sizes.
- Capture automation reaches the terminal success state and exits through the same automatic-close behavior.

---

### Task 1: Terminal completion and clean automatic shutdown

**Files:**
- Create: `src/ui/CompletionExitSequence.h`
- Modify: `src/ui/LoaderUI.h`
- Modify: `src/ui/LoaderUI.cpp`
- Modify: `src/ui/screens/ProductLoadOverlay.h`
- Modify: `src/ui/screens/ProductLoadOverlay.cpp`
- Modify: `src/platform/Win32Application.cpp`
- Modify: `src/platform/FrameCapture.h`
- Modify: `src/platform/FrameCapture.cpp`
- Test: `tests/AnimationTests.cpp`
- Test: `tests/AssetCaptureTests.cpp`

**Interfaces:**
- Produces: `CompletionExitSequence::update(OperationState, float)`, `opacity()`, and `close_requested()`.
- Produces: `LoaderFrameResult { blur_strength, window_opacity, close_requested }` from `LoaderUI::render`.

- [x] Add failing tests for terminal hold/fade timing, success/failure parity, reset behavior, and the sixth `product-success.png` capture.
- [x] Run the focused tests and verify they fail for the missing sequence and old five-frame contract.
- [x] Implement the minimal terminal sequence, remove the completion button, and route window opacity/close through Win32Application.
- [x] Extend deterministic capture through terminal success and let automatic shutdown close the app.
- [x] Run focused and complete tests until green.

### Task 2: Reference-matched typography, buttons, icons, and dark surfaces

**Files:**
- Modify: `src/ui/UiContracts.h`
- Modify: `src/ui/Theme.h`
- Modify: `src/ui/Theme.cpp`
- Modify: `src/ui/Components.cpp`
- Modify: `src/ui/LoaderUI.cpp`
- Modify: `src/ui/screens/AuthScreen.cpp`
- Modify: `src/ui/screens/ProductScreen.cpp`
- Modify: `src/ui/screens/SelectedProductPanel.cpp`
- Modify: `src/ui/screens/ProductLoadOverlay.cpp`
- Test: `tests/UiContractTests.cpp`

**Interfaces:**
- Produces: centralized reference-derived typography sizes, darker button corner colors, and rounded shared-button treatment.
- Consumes: the current supplied-icon vector geometry and existing screen layouts without structural changes.

- [x] Add failing typography/button token assertions before changing production values.
- [x] Increase the login heading/brand hierarchy and use semibold button text consistently.
- [x] Darken/desaturate purple buttons, add smooth bottom-right gray-purple shading, and slightly increase corner rounding.
- [x] Add a restrained four-corner dark background blend and correct loading/metadata text sizes without changing composition.
- [x] Build and capture all six states, compare at native scale, and make only measured corrections.

### Task 3: Version, release verification, commit, and push

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `src/app/Configuration.cpp`
- Modify: `tests/AppModelTests.cpp`
- Modify: `README.md`
- Modify: `.gitignore`

**Interfaces:**
- Produces: Serein `0.4.0`, documented six-frame capture flow, and a clean source commit.

- [x] Add the failing `0.4.0` configuration assertion, then update project/configuration/docs.
- [x] Ignore generated `review/` output while retaining source assets.
- [x] Configure and build Debug and Release.
- [x] Run the complete Release CTest suite and the ImGui scope checker.
- [x] Run the deterministic workflow to prove terminal capture and automatic process exit.
- [x] Review the staged source/assets list, commit the completed Design 1 work once, and push `loader-designs` to `origin`.
