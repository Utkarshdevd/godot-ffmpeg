# 🎥 Godot FFmpeg Player

[![Build](https://github.com/Utkarshdevd/godot-ffmpeg/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/Utkarshdevd/godot-ffmpeg/actions/workflows/build.yml)
![C++](https://img.shields.io/badge/C++-20-blue.svg?style=flat&logo=c%2B%2B)
![Godot](https://img.shields.io/badge/Godot-4.2-478cbf?style=flat&logo=godot-engine&logoColor=white)
![License](https://img.shields.io/github/license/Utkarshdevd/godot-ffmpeg)

A high-performance video player extension for Godot 4, built with C++ and FFmpeg.

- **godot-cpp** bindings, **SCons** build
- **macOS** (Apple Silicon and Intel), **Linux**, **Windows**; FFmpeg from Homebrew / system / CI
- Core **demuxer** and unified **logger**; headless tests for `src/core` without the Godot Editor

---

## Prerequisites

- **Godot 4.2+**
- **Python 3.8+** and **SCons 4.x**
- **FFmpeg** (headers and libs): install via Homebrew on macOS, or set `FFMPEG_PREFIX` to your FFmpeg prefix
- **godot-cpp** as a submodule (see build steps below)

---

## Build strategy and order

1. **Get godot-cpp** (submodule) and build it first:

   ```bash
   git submodule update --init --recursive
   cd godot-cpp
   scons target=template_debug    # and optionally target=template_release
   # On Intel Mac, use arch=x86_64 to match your FFmpeg:
   scons target=template_debug arch=x86_64
   cd ..
   ```

2. **Build the extension** from the repo root:

   ```bash
   # Intel Mac (x86_64)
   scons target=template_debug arch=x86_64
   scons target=template_release arch=x86_64

   # Apple Silicon (arm64) or universal
   scons target=template_debug
   scons target=template_release
   ```

3. **Output:** Compiled `.dylib` (or platform equivalent) is installed under **`addons/godot_av/bin/`** (e.g. `addons/godot_av/bin/macos/` for x86_64, `addons/godot_av/bin/macos.arm64/` for arm64).

**Optional:** Use **ccache** for faster rebuilds:

```bash
scons target=template_debug arch=x86_64 use_ccache=yes
```

---

## Using the extension in a Godot project

1. Copy the **`addons/godot_av`** folder into your project (so you have `YourProject/addons/godot_av/` with `godot_av.gdextension` and `bin/` containing the built library for your OS/arch).
2. Open the project in Godot 4.x. Ensure the project root is the folder that contains `addons/`.
3. Add an **AVTestNode** to a scene (Add Child Node → search for **AVTestNode**).
4. Run the scene. In the Output panel you should see: `[godot_av] FFmpeg version: <version>`.

You only need the **debug** library for editor/Play; use the **release** library when exporting a release build.

---

## Testing & Quality Assurance

- **Core Layer Isolation:** Unit tests run on `src/core` logic (e.g. the logger) **without** the Godot Editor. Tests are built without defining `GODOT_EXTENSION`, so the logger uses Terminal Mode (`std::cout`).
- **Running Tests:** Build and run all tests in one step:

  ```bash
  scons run_tests=yes
  ```

  Or build the test binary only, then run it manually:

  ```bash
  scons bin/test_logger
  ./bin/test_logger          # macOS/Linux
  .\bin\test_logger.exe     # Windows
  ```

- **Sanitizers (Local Workflow):** For local debugging, use SCons sanitizer flags:
  - `scons target=template_debug` — standard debug build
  - `scons target=template_debug sanitize=address` — AddressSanitizer (memory leaks, buffer overflows)
  - `scons target=template_debug sanitize=thread` — ThreadSanitizer (race conditions)
  - **Note:** Run sanitizer-built tests via the headless test binaries only; do **not** load sanitizer-built libraries in the Godot Editor.
- **CI:** On every push or PR to `main`, the workflow builds the extension (macOS, Linux, Windows) and then runs all tests (`scons run_tests=yes`). The pipeline fails if any test fails.

---

## Test assets (optional)

- **Generate Assets:** Test video assets are not committed. Generate them once (requires FFmpeg on PATH):

  ```bash
  python3 tests/generate_assets.py
  ```

---

## Errors faced and fixed

| Issue | Cause | Fix |
| ----- | ----- | --- |
| **`Class must declare 'static void _bind_methods'`** | godot-cpp requires every registered class to define its own `_bind_methods()`. | Added `static void _bind_methods();` in `AVTestNode` (header) and an empty `void AVTestNode::_bind_methods() {}` in the .cpp. |
| **`'godot_cpp/classes/node.hpp' file not found`** | IDE/compiler couldn’t find generated godot-cpp bindings. `node.hpp` is generated under `godot-cpp/gen/include/`. | SConstruct appends `godot-cpp/include` and `godot-cpp/gen/include` to `CPPPATH`. For the IDE, `.vscode/c_cpp_properties.json` was added with those include paths. |
| **`ld: ignoring file ... libavutil.dylib: found architecture 'x86_64', required architecture 'arm64'`** | On Apple Silicon, `brew --prefix ffmpeg` pointed at Intel Homebrew (`/usr/local`), so linker got x86_64 libs while building for arm64. | Prefer `/opt/homebrew/opt/ffmpeg` on macOS when it exists; reject `/usr/local` only when building for `arm64` or `universal`. |
| **Intel Mac: same linker error** | Build defaulted to arm64/universal; Intel FFmpeg is x86_64-only. | Build with **`arch=x86_64`** on Intel Mac so the linker uses `/usr/local/opt/ffmpeg` (x86_64). SConstruct only rejects `/usr/local` when `env["arch"]` is `arm64` or `universal`. |
| **`ERROR: FFmpeg at /usr/local/opt/ffmpeg is from Intel ... This build targets arm64`** when running `arch=x86_64` | Rejection of `/usr/local` ran before SConscript, so `env["arch"]` wasn’t set yet and the check was wrong. | Moved the “reject Intel Homebrew” check to **after** `SConscript("godot-cpp/SConstruct")` so we only reject when `env["arch"]` is `arm64` or `universal`. |
| **`GDExtension dynamic library not found`** / **`res://addons/godot_av/...`** | Project used an **`addons/`** folder but the .gdextension and SConstruct used **`addon/`**. | Switched everything to **`addons/godot_av`**: paths in `godot_av.gdextension`, `projectdir` in SConstruct, and moved the addon from `addon/godot_av` to `addons/godot_av`. |
| **Extension not loading in another project** | Addon was copied but the built `.dylib` was missing under `addons/godot_av/bin/...`, or project was opened from the wrong root. | Copy the entire **`addons/godot_av`** folder (including `bin/<platform>/<lib>`). Open the project from the directory that contains `addons/`. |

---

## Project layout

```text
godot-ffmpeg/
├── .github/workflows/     # CI (build + tests on push/PR to main)
├── addons/godot_av/      # GDExtension addon
│   ├── godot_av.gdextension
│   └── bin/              # Built libs (gitignored)
├── godot-cpp/            # Submodule
├── src/
│   ├── register_types.{h,cpp}
│   ├── av_test_node.{h,cpp}
│   ├── core/
│   │   └── logger/       # Unified logging (Godot + headless)
│   │       ├── logger.hpp
│   │       └── logger.cpp
│   └── gen/              # Generated (gitignored)
├── tests/
│   ├── test_logger.cpp   # Logger test harness
│   └── generate_assets.py
├── bin/                  # Test binaries (e.g. test_logger) (gitignored)
├── SConstruct
├── custom.py
└── README.md
```

---

## License

See [LICENSE](LICENSE).
