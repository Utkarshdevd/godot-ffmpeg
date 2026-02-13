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
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()

if not (os.path.isdir("godot-cpp") and os.listdir("godot-cpp")):
    print("godot-cpp is not available. Run: git submodule update --init --recursive")
    sys.exit(1)

env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

# On macOS, Intel Homebrew (/usr/local) is x86_64 only — reject it when building for arm64 or universal
if sys.platform == "darwin" and brew_prefix.startswith("/usr/local") and env["arch"] in ("arm64", "universal"):
    print("")
    print("ERROR: FFmpeg at {} is x86_64 only. This build targets {}.".format(brew_prefix, env["arch"]))
    print("Use arch=x86_64 for Intel Mac, or install arm64 FFmpeg and set FFMPEG_PREFIX=/opt/homebrew/opt/ffmpeg")
    print("")
    sys.exit(1)

# ccache: wrap CC and CXX if enabled and ccache is available
if use_ccache:
    ccache = shutil.which("ccache")
    if ccache:
        for var in ("CC", "CXX"):
            if env.get(var):
                env[var] = ccache + " " + env[var]
        print("Using ccache for compilation.")
    else:
        print("use_ccache=yes but ccache not found in PATH; building without ccache.")

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

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

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
