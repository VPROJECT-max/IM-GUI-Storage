# Serein Visual Corrections Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Bring Loader Design 1 visually and behaviorally close to the supplied reference captures while preserving the current DirectX 11, Dear ImGui, and application architecture.

**Architecture:** Keep the existing screen classes and model/service boundaries. Add deterministic, duration-based animation helpers, render the supplied Lucide icon geometry through reusable ImGui draw-list helpers, and generate a genuinely blurred companion texture for each product artwork so card footers can reveal softened artwork under a translucent tint.

**Tech Stack:** C++20, DirectX 11, Dear ImGui v1.92.0, WIC, CMake/CTest.

**Spec:** User-supplied “Serein — Continue Existing Project: Visual Corrections” brief dated 2026-10-17.

## Global Constraints

- Work only in `Loader Designs/1`; do not modify Design 2 or Menu Designs.
- Keep the fixed application dimensions and existing architecture.
- Keep the product artwork and the Serein brand name.
- Keep Settings but do not expand or redesign it.
- Do not add injection, cheating, or remote-authentication behavior.
- Update the SemVer version consistently.
- Do not commit or push.
- Validate with Release build, CTest, deterministic runtime captures, and image inspection.

## Review Focus

- Low and high frame rates must produce the same 700–900 ms panel motion and 2.5–3.5 second loading duration.
- Closing and switching selections must retain outgoing content until the slide completes instead of snapping.
- The Remember Me hit target must include both the box and the complete label.
- Blurred card footers must preserve visible artwork color while keeping text readable.
- Missing artwork or generated blur resources must fall back safely without crashing or obscuring controls.

---

### Task 1: Lock the timing and interaction contracts

**Files:**
- Modify: `src/ui/Animation.h`
- Modify: `src/ui/UiContracts.h`
- Modify: `src/app/Configuration.h`
- Modify: `src/app/Configuration.cpp`
- Modify: `src/integration/MockServices.h`
- Test: `tests/AnimationTests.cpp`
- Test: `tests/UiContractTests.cpp`
- Test: `tests/AppModelTests.cpp`

**Interfaces:**
- Produces: `animation::advance_toward(float current, float target, float duration_seconds, float delta_seconds)` and visual timing constants used by ProductScreen.

- [ ] Add failing tests for frame-rate-independent 820 ms progress, full Remember Me hit width, 3-second discovery, 3-second product loading, and version `0.3.0`.
- [ ] Rebuild the affected tests and confirm the new assertions fail for the expected old values or missing helpers.
- [ ] Implement the minimal timing/configuration contracts.
- [ ] Rebuild and confirm all contract tests pass.

### Task 2: Add reusable fidelity rendering primitives

**Files:**
- Modify: `src/render/TextureStore.h`
- Modify: `src/render/TextureStore.cpp`
- Modify: `src/ui/Components.h`
- Modify: `src/ui/Components.cpp`
- Test: `tests/PlatformContractTests.cpp`

**Interfaces:**
- Produces: a blurred shader-resource view alongside each loaded texture, gradient text drawing, supplied-icon vector drawing, and refined gradient buttons.

- [ ] Add a failing platform contract asserting that loaded textures expose a blurred companion view contract.
- [ ] Implement separable Gaussian-style RGBA blur during WIC texture upload and preserve a safe normal-view fallback.
- [ ] Implement horizontal gradient text and reusable Lucide-derived `status`, `calendar`, `user`, `settings`, `load`, `product`, and `close` icon drawing.
- [ ] Refine primary/secondary button gradients, border, hover, and restrained shadow layers.
- [ ] Rebuild and confirm platform and ImGui static checks pass.

### Task 3: Correct the login screen

**Files:**
- Modify: `src/ui/screens/AuthScreen.cpp`

**Interfaces:**
- Consumes: gradient text and button primitives from Task 2.

- [ ] Expand the Remember Me invisible hit area to cover checkbox and label and preserve the original visuals.
- [ ] Render “Serein” with a left-purple to dark-purple horizontal fade and match the reference typography/spacing.
- [ ] Add restrained surface depth to the login pane and use the shared gradient button treatment.
- [ ] Build and capture `login.png`; inspect it beside the original and tune only measured mismatches.

### Task 4: Correct product cards and panel motion

**Files:**
- Modify: `src/ui/screens/ProductScreen.h`
- Modify: `src/ui/screens/ProductScreen.cpp`

**Interfaces:**
- Consumes: blurred texture views, supplied-icon drawing, duration-based animation helper.
- Produces: retained display selection during close/switch and an eased 820 ms slide.

- [ ] Replace opaque card footers with clipped blurred artwork plus translucent/tinted gradients.
- [ ] Apply subtle loader-surface shading and refined product button gradients.
- [ ] Implement opening, closing, and selection-switch state without snapping or blocking the render thread.
- [ ] Capture `products-loading.png` and `products.png`; inspect and tune footer tint, image visibility, spacing, and transition readability.

### Task 5: Correct the selected panel and product-loading overlay

**Files:**
- Modify: `src/ui/screens/SelectedProductPanel.h`
- Modify: `src/ui/screens/SelectedProductPanel.cpp`
- Modify: `src/ui/screens/ProductLoadOverlay.cpp`
- Modify: `src/platform/FrameCapture.h`
- Modify: `src/platform/FrameCapture.cpp`
- Test: `tests/AssetCaptureTests.cpp`

**Interfaces:**
- Consumes: retained product selection, icon drawing, gradient buttons.
- Produces: selected panel without Clean, transparent standalone close icon, lower-section tint, and deterministic `product-loading.png` capture.

- [ ] Add a failing capture contract for five named review frames including product loading.
- [ ] Rebalance selected-panel typography and metadata rows, add the supplied status/calendar/user/settings/load icons, and remove the visible X container.
- [ ] Add the subtle translucent lower action section and preserve only Settings and Load.
- [ ] Refine the Loading Product overlay entrance/exit presentation while keeping the operation non-blocking.
- [ ] Extend deterministic capture timing so the 820 ms panel is fully settled before capture, then capture product loading while it remains active.

### Task 6: Release validation and visual iteration

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `README.md`
- Modify: `LICENSES.md` only if icon attribution needs clarification.

- [ ] Update project and documented version to `0.3.0` and describe the new capture set accurately.
- [ ] Configure and build Release from current sources.
- [ ] Run the complete CTest suite with zero failures.
- [ ] Run the ImGui balance/static checker on all changed UI sources.
- [ ] Generate fresh runtime captures and compare login, discovery, products, selected, and product-loading states to the supplied references.
- [ ] Make bounded visual adjustments, rebuild, recapture, and report any remaining differences honestly.
