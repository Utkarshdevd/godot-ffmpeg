# godot-ffmpeg: AI Agent Directives & Architecture

## Project Overview

**godot-ffmpeg** is a Godot 4.x GDExtension written in **C++20** that integrates FFmpeg to provide high-performance video playback.

* **Initial Scope:** MP4/H.264
* **Expansion:** AV1, HEVC, VP9
* **Goal:** A cross-platform, threaded, high-resolution screen recording viewer and media backend suitable for 4K+ UI playback.

**Key Priorities:**

* Correct Architecture
* Thread Safety
* Performance
* Testability
* Clean Separation of Concerns
* CI Validation (Cross-Platform)

---

## 1. Architectural Principles

### 1. Layer Separation (Strict)

The project adheres to a strict boundary between the core logic and the engine wrapper.

| Layer | Path | Constraints | Responsibilities |
| :--- | :--- | :--- | :--- |
| **Core Layer** | `src/core/`, `src/media/` | Pure C++; **NO** Godot headers/types; **NO** Godot object manipulation | FrameQueue; Clock; Decoder Wrapper; Demux Logic; Sync Logic |
| **Godot Layer** | `src/godot/` | Thin wrapper around Core; **NO** decoding logic | Texture Upload; Godot Node Integration; Signal Emission |

### 2. Threading Model

Decoding must strictly occur on a background thread to prevent blocking the engine.

**Expected Data Flow:**
`Decode Thread` $\rightarrow$ `FrameQueue` $\rightarrow$ `Main Thread (Godot)`

**Rules:**

* **Main Thread:** Only Godot objects may be accessed here.
* **FrameQueue:** Must be thread-safe.
* **Shutdown:** Clean thread shutdown via `std::jthread` and stop tokens.
* **Prohibited:** No busy-wait loops; no blocking the main thread.

### 3. Namespace Discipline

* All code must reside in: `namespace godot_ffmpeg`
* **NO** global symbols.
* **NEVER** add code inside `namespace godot`.

### 4. Logging Discipline

All logging must use the centralized logger at `src/core/log.hpp`.

* **Forbidden:** `std::cout`, `printf`, `UtilityFunctions::print`
* **Required Macros:** `LOG_DEBUG`, `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`
* **Stripping:** Debug logs must be compile-time stripped in release builds via `GODOT_FFMPEG_DEBUG`.

### 5. Performance Principles

Targeting high-resolution content (4K+).

* **Zero-Copy:** Avoid unnecessary frame copies.
* **Allocation:** Avoid dynamic memory allocations inside the decode loop.
* **Bounded Queue:** FrameQueue must have a fixed capacity.
* **Future Proofing:** Prepare for GPU YUV conversion and hardware decode.
* **Regressions:** Unacceptable without significant justification.

---

## 2. Testing Discipline

### Core Code Testability

Everything in `src/core/` and `src/media/` must be unit-testable **independently** of Godot.

### Testing Rules

* **Location:** Tests live in `tests/`.
* **Requirement:** All new features affecting core logic must include unit tests.
* **Gatekeeping:** All tests must pass before merging. No feature merges without green CI.

### CI Expectations

CI validates cross-platform compilation (macOS, Linux, Windows), FFmpeg linking, unit tests, and both Debug/Release builds. **Never merge red CI.**

---

## 3. Workflow & Strategy

### Branching Strategy

* `main`: Always stable.
* `feature/*`: Feature code + tests. Deleted after merge.
* `fix/*`: Bug fixes.
* **Note:** Do NOT create separate test-only branches.

### Codec Strategy

The decoder must remain **codec-agnostic**. Codec selection relies on FFmpeg stream metadata.

* **Minimum:** H.264, AV1
* **Future:** HEVC, VP9

---

## 4. Debugging & Constraints

### Debugging Guidelines

* Always debug `template_debug` builds.
* Use centralized logging.
* Prefer assertions in debug builds.
* **Do not** interact with Godot objects from the background thread.

### Forbidden Practices

1. No Godot includes in the core decoder layer.
2. No global mutable state.
3. No raw pointers without ownership clarity.
4. No unbounded queues.
5. No ad-hoc logging systems.

---

## 5. Future Roadmap

1. Decode core & Thread-safe FrameQueue
2. Minimal texture upload
3. Audio + Sync
4. Seek support
5. GPU YUV conversion
6. Hardware decode
7. Android/iOS support

---

## AI Agent Expectations

When generating code for `godot-ffmpeg`, you must:

1. **Respect Namespace:** Use `godot_ffmpeg`.
2. **Respect Layering:** strictly separate `src/core` (Pure C++) from `src/godot`.
3. **Use Centralized Logging:** Use `log.hpp` macros.
4. **Ensure Testability:** Write components that can be tested without Godot.
5. **Thread Safety:** Use strict ownership and thread-safe queues.

**Decision Heuristic:**
If uncertain, favor **Simplicity**, **Determinism**, **Testability**, and **Clean Layering**.

> **Final Note:** This is a robust media backend, not a prototype. Correct architecture is more important than speed of implementation.
