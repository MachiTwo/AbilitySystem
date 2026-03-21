#!/usr/bin/env python
import os
import sys

from methods import print_error


libname = "ability_system"
projectdir = "demo"

# --- Godot-style Environment Setup ---
# Inherit system environment to find site-packages and other user-installed libs
localEnv = Environment(ENV=os.environ.copy(), tools=["default"])

# Avoid issues when building with different versions of python
import pickle

localEnv.SConsignFile(
    File("#.sconsign{0}.dblite".format(pickle.HIGHEST_PROTOCOL)).abspath
)

# Use MD5 decider for absolute reliability in incremental builds.
localEnv.Decider("MD5")

# Prepend PATH to ensure system compilers are found consistently
localEnv.PrependENVPath("PATH", os.getenv("PATH"))

# Auto-detect CPU cores for parallel build if -j is not specified
if localEnv.GetOption("num_jobs") <= 1:
    cpu_count = os.cpu_count()
    if cpu_count and cpu_count > 1:
        localEnv.SetOption("num_jobs", cpu_count - 1)
        print(f"Auto-detected {cpu_count} CPU cores. Using {cpu_count - 1} for build.")

# --- SCons Cache ---
scons_cache_path = os.environ.get("SCONS_CACHE")
if not scons_cache_path and os.path.exists(".scons_cache"):
    scons_cache_path = os.path.abspath(".scons_cache")

if scons_cache_path:
    CacheDir(scons_cache_path)
    localEnv.NoCache(localEnv.Value(os.environ.get("PATH")))
    print(f"Using SCons cache at: {scons_cache_path}")

# --- Compilation Database (for IDEs) ---
if "compiledb" in ARGUMENTS:
    localEnv.Tool("compilation_db")
    localEnv.Alias("compiledb", localEnv.CompilationDatabase())

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()

if not (os.path.isdir("godot-cpp") and os.listdir("godot-cpp")):
    print(
        "godot-cpp is not available within this folder, as Git submodules haven't been initialized."
    )
    sys.exit(1)

# --- godot-cpp Automatic Patching (CI Resilience) ---
gcpp_tool_path = "godot-cpp/tools/godotcpp.py"
if os.path.exists(gcpp_tool_path):
    with open(gcpp_tool_path, "r") as f:
        content = f.read()
    if "env.NoCache(library)" in content:
        new_content = content.replace("env.NoCache(library)", "# env.NoCache(library)")
    if "env.NoCache(bindings)" in content:
        new_content = new_content.replace(
            "env.NoCache(bindings)", "# env.NoCache(bindings)"
        )
    if new_content != content:
        print("[SCONSTRUCT] Patching godot-cpp/tools/godotcpp.py to enable caching...")
        with open(gcpp_tool_path, "w") as f:
            f.write(new_content)

gcpp_sconstruct_path = "godot-cpp/SConstruct"
if os.path.exists(gcpp_sconstruct_path):
    with open(gcpp_sconstruct_path, "r") as f:
        content = f.read()
    # We check for a marker to avoid double patching and ensure our Decider is the one used.
    if "[GCPP_MD5_PATCH]" not in content:
        print("[SCONSTRUCT] Patching godot-cpp/SConstruct to enforce MD5 decider...")
        # Force MD5 at the very top of the file
        patch = '# [GCPP_MD5_PATCH]\nDecider("MD5")\nif "env" in globals() or "env" in locals():\n    env.Decider("MD5")\n'
        content = patch + content
        with open(gcpp_sconstruct_path, "w") as f:
            f.write(content)

if scons_cache_path:
    os.environ["SCONS_CACHE"] = scons_cache_path

env = SConscript("godot-cpp/SConstruct", {"env": env.Clone(), "customs": customs})

env.Append(CPPPATH=[".", "src"])
env.Append(CPPDEFINES=["ABILITY_SYSTEM_GDEXTENSION"])

# Configure object folder mapping (VariantDir)
env.VariantDir("src/bin", "src", duplicate=0)

# Collect all source files
sources = Glob("src/bin/*.cpp")
sources += Glob("src/bin/core/*.cpp")
sources += Glob("src/bin/resources/*.cpp")
sources += Glob("src/bin/scene/*.cpp")

if env["target"] in ["editor"]:
    sources += Glob("src/bin/editor/*.cpp")
    try:
        doc_data = env.GodotCPPDocData(
            "src/bin/gen/doc_data.gen.cpp", source=Glob("src/doc_classes/*.xml")
        )
        sources.append(doc_data)
    except AttributeError:
        pass

# --- Target Definition ---
suffix = env["suffix"].replace(".dev", "").replace(".universal", "")
lib_filename = "{}{}{}{}".format(
    env.subst("$SHLIBPREFIX"), libname, suffix, env.subst("$SHLIBSUFFIX")
)

library = env.SharedLibrary(
    "addons/ability_system/bin/{}".format(lib_filename),
    source=sources,
)

# --- Post-Build / Sync ---
post_build_script = os.path.join("utility", "post_build.py")
if os.path.exists(post_build_script):
    post_build_action = env.Command(
        "post_build_dummy", library, f"{sys.executable} {post_build_script}"
    )
    env.AlwaysBuild(post_build_action)
    env.Depends(post_build_action, library)
    Default(library, post_build_action)
else:
    Default(library)
