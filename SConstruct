#!/usr/bin/env python
import os
import shutil
import sys

libname = "godot_av"
projectdir = "addons/godot_av"

# FFmpeg via Homebrew — use arm64 prefix on macOS when present (avoids x86_64 under Rosetta)
if os.environ.get("FFMPEG_PREFIX"):
    brew_prefix = os.environ.get("FFMPEG_PREFIX").strip()
elif sys.platform == "darwin" and os.path.isdir("/opt/homebrew/opt/ffmpeg"):
    brew_prefix = "/opt/homebrew/opt/ffmpeg"
elif sys.platform == "darwin":
    brew_prefix = os.popen("brew --prefix ffmpeg").read().strip()
elif sys.platform == "win32":
    # Windows: expect FFMPEG_PREFIX to be set via environment (CI sets this)
    brew_prefix = os.environ.get("FFMPEG_PREFIX", "C:\\ProgramData\\chocolatey\\lib\\ffmpeg\\tools")
else:
    # Linux: default to /usr
    brew_prefix = os.environ.get("FFMPEG_PREFIX", "/usr")

ffmpeg_include = os.path.join(brew_prefix, "include")
ffmpeg_lib = os.path.join(brew_prefix, "lib")
print("Using FFmpeg at: {}".format(brew_prefix))

localEnv = Environment(tools=["default"], PLATFORM="")

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

# ccache: use_ccache=yes to enable, or set USE_CCACHE=1 in environment
use_ccache = ARGUMENTS.get("use_ccache", os.environ.get("USE_CCACHE", "no")).lower() in ("1", "yes", "true")

opts = Variables(customs, ARGUMENTS)
opts.Add(BoolVariable("use_ccache", "Use ccache for compilation", use_ccache))
opts.Add(
    EnumVariable(
        "sanitize",
        "Enable compiler sanitizers (address, thread, undefined)",
        "none",
        allowed_values=("none", "address", "thread", "undefined"),
    )
)
opts.Add(BoolVariable("run_tests", "Build and run tests after building", False))
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()

if not (os.path.isdir("godot-cpp") and os.listdir("godot-cpp")):
    print("godot-cpp is not available. Run: git submodule update --init --recursive")
    sys.exit(1)

# If user passed target=test_logger (or similar), godot-cpp only accepts editor/template_debug/template_release.
# Override so the submodule builds with template_debug; our test targets are built from Default().
_godot_target = ARGUMENTS.get("target")
if _godot_target and _godot_target not in ("editor", "template_debug", "template_release"):
    ARGUMENTS["target"] = "template_debug"

env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

# On macOS, Intel Homebrew (/usr/local) is x86_64 only — reject it when building for arm64 or universal
if sys.platform == "darwin" and brew_prefix.startswith("/usr/local") and env["arch"] in ("arm64", "universal"):
    print("")
    print("ERROR: FFmpeg at {} is x86_64 only. This build targets {}.".format(brew_prefix, env["arch"]))
    print("Use arch=x86_64 for Intel Mac, or install arm64 FFmpeg and set FFMPEG_PREFIX=/opt/homebrew/opt/ffmpeg")
    print("")
    sys.exit(1)

# ccache: wrap CC and CXX if enabled (use_ccache=yes or CCACHE in env) so sanitizer flag changes trigger cache-miss
_use_ccache = use_ccache or os.environ.get("CCACHE") is not None
if _use_ccache:
    ccache = shutil.which("ccache")
    if ccache:
        for var in ("CC", "CXX"):
            if env.get(var) and "ccache" not in str(env.get(var, "")):
                env[var] = "ccache " + env[var]
        print("Using ccache for compilation.")
    else:
        print("use_ccache=yes or CCACHE set but ccache not found in PATH; building without ccache.")

# Ensure godot-cpp headers (including generated bindings in gen/include) are found
project_root = env.Dir("#").abspath
godot_cpp_include = os.path.join(project_root, "godot-cpp", "include")
godot_cpp_gen_include = os.path.join(project_root, "godot-cpp", "gen", "include")
env.Append(CPPPATH=[godot_cpp_include, godot_cpp_gen_include])

# C++20 (MSVC uses /std:c++20, GCC/Clang use -std=c++20)
# Use sys.platform so we always pass the right flag on Windows (is_msvc may not be set when building extension)
_on_windows = sys.platform in ("win32", "msys", "cygwin")
if _on_windows:
    env.Append(CXXFLAGS=["/std:c++20"])
else:
    env.Append(CXXFLAGS=["-std=c++20"])

# FFmpeg paths and libs
env.Append(CPPPATH=[ffmpeg_include])
env.Append(LIBPATH=[ffmpeg_lib])
env.Append(LIBS=["avformat", "avcodec", "avutil", "swscale", "swresample"])

# Runtime rpath so Godot finds FFmpeg dylibs (macOS/Linux only)
if sys.platform != "win32":
    env.Append(LINKFLAGS=["-Wl,-rpath," + ffmpeg_lib])

# Sanitizers (GCC/Clang only; for test_runner and main library debugging)
if not _on_windows and env.get("sanitize", "none") != "none":
    san = env["sanitize"]
    env.Append(CCFLAGS=["-fsanitize=%s" % san, "-g", "-fno-omit-frame-pointer"])
    env.Append(LINKFLAGS=["-fsanitize=%s" % san])
    print("WARNING: Building with Sanitizer: %s ..." % san)

env.Append(CPPPATH=["src/"])

# Define GODOT_EXTENSION for the main GDExtension build
# (test_runner builds will not define this, allowing std::cout logging)
env.Append(CPPDEFINES=["GODOT_EXTENSION"])

# Collect source files (including subdirectories)
sources = Glob("src/*.cpp") + Glob("src/core/*.cpp") + Glob("src/core/logger/*.cpp")

if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        pass

suffix = env["suffix"].replace(".dev", "").replace(".universal", "")
lib_filename = "{}{}{}{}".format(env.subst("$SHLIBPREFIX"), libname, suffix, env.subst("$SHLIBSUFFIX"))

library = env.SharedLibrary(
    "bin/{}/{}".format(env["platform"], lib_filename),
    source=sources,
)

copy = env.Install("{}/bin/{}".format(projectdir, env["platform"]), library)

Default(library)
Default(copy)

# Test logger build target (headless, no GODOT_EXTENSION)
test_env = env.Clone()
# Remove GODOT_EXTENSION define for test builds
test_env["CPPDEFINES"] = [d for d in test_env.get("CPPDEFINES", []) if d != "GODOT_EXTENSION"]
# Remove Godot-specific libraries and paths for test builds
test_env["LIBS"] = [lib for lib in test_env.get("LIBS", []) if not str(lib).startswith("godot")]
test_env["CPPPATH"] = [p for p in test_env.get("CPPPATH", []) if "godot-cpp" not in str(p)]

test_logger = test_env.Program(
    "bin/test_logger",
    source=["tests/test_logger.cpp", "src/core/logger/logger.cpp"],
)

Default(test_logger)

# Test demuxer (links against FFmpeg libs)
test_demuxer = test_env.Program(
    "bin/test_demuxer",
    source=["tests/test_demuxer.cpp"],
    LIBS=["avformat", "avcodec", "avutil"],
)

Default(test_demuxer)

# Test runner: build and run all tests if run_tests=yes
run_tests_flag = ARGUMENTS.get("run_tests", "no").lower() in ("1", "yes", "true")

if run_tests_flag:
    import subprocess
    
    def run_tests(target, source, env):
        """Build tests and run them."""
        # Test binaries - check both with and without .exe extension for Windows
        test_binaries = []
        base_name = "bin/test_logger"
        if sys.platform == "win32":
            test_binaries = [base_name + ".exe", base_name]
        else:
            test_binaries = [base_name]
        
        failed_tests = []
        
        for test_bin in test_binaries:
            if not os.path.exists(test_bin):
                continue  # Try next variant
            
            print("\n" + "="*60)
            print("Running: {}".format(test_bin))
            print("="*60)
            
            try:
                # Use absolute path for cross-platform compatibility
                test_path = os.path.abspath(test_bin)
                result = subprocess.run([test_path], cwd=os.path.dirname(test_path) or ".", 
                                       capture_output=False, text=True)
                if result.returncode != 0:
                    print("FAILED: {} exited with code {}".format(test_bin, result.returncode))
                    failed_tests.append(test_bin)
                else:
                    print("PASSED: {}".format(test_bin))
                break  # Found and ran the test, exit loop
            except Exception as e:
                print("ERROR: Failed to run {}: {}".format(test_bin, e))
                failed_tests.append(test_bin)
        
        # If we didn't find any test binary, report error
        if not any(os.path.exists(tb) for tb in test_binaries):
            print("ERROR: Test binary not found. Expected one of: {}".format(test_binaries))
            failed_tests.append("test_logger")
        
        if failed_tests:
            print("\n" + "="*60)
            print("TEST SUMMARY: {} test(s) failed".format(len(failed_tests)))
            print("="*60)
            sys.exit(1)
        else:
            print("\n" + "="*60)
            print("TEST SUMMARY: All tests passed!")
            print("="*60)
    
    # Create an alias that runs tests
    test_runner = localEnv.Alias("test", [test_logger], run_tests)
    AlwaysBuild(test_runner)
    Default(test_runner)
