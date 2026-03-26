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
    new_content = content.replace("env.NoCache(library)", "# env.NoCache(library)")
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
    if 'Decider("MD5")' not in content:
        print("[SCONSTRUCT] Patching godot-cpp/SConstruct to enforce MD5 decider...")
        lines = content.splitlines()
        insert_idx = 0
        for i, line in enumerate(lines):
            if (
                not line.startswith("#")
                and not line.startswith("import")
                and not line.startswith("sys.path")
                and line.strip()
            ):
                insert_idx = i
                break
        lines.insert(
            insert_idx,
            'Decider("MD5")\nif "env" in globals() or "env" in locals():\n    env.Decider("MD5")\n',
        )
        with open(gcpp_sconstruct_path, "w") as f:
            f.write("\n".join(lines))

env = SConscript("godot-cpp/SConstruct", {"env": env.Clone(), "customs": customs})

env.Append(CPPPATH=[".", "src"])
env.Append(CPPDEFINES=["ABILITY_SYSTEM_GDEXTENSION"])

# Configure LimboAI Integration if available
if os.path.isdir("limboai"):
    print("[SCONSTRUCT] LimboAI detected. Integrating sources...")
    env.Append(CPPPATH=["limboai"])
    env.Append(CPPDEFINES=["LIMBOAI_GDEXTENSION"])

    # --- LimboAI Automatic Patching (CI Resilience) ---
    # Patch limboai/compat/variant.cpp to fix include-order bug:
    # variant.h transitively includes variant.hpp -> hashfuncs.hpp which
    # forward-declares Ref<>, shadowing the full template from ref.hpp.
    # We insert ref.hpp + ref_counted.hpp BEFORE variant.h to fix this.
    limboai_variant_cpp = os.path.join("limboai", "compat", "variant.cpp")
    if os.path.exists(limboai_variant_cpp):
        with open(limboai_variant_cpp, "r") as f:
            variant_content = f.read()
        if "ABILITY_SYSTEM_GDEXTENSION" not in variant_content:
            patched = variant_content.replace(
                '#include "variant.h"',
                "#if defined(ABILITY_SYSTEM_GDEXTENSION)\n"
                "#include <godot_cpp/classes/ref.hpp>\n"
                "#include <godot_cpp/classes/ref_counted.hpp>\n"
                "#endif // ABILITY_SYSTEM_GDEXTENSION\n"
                '#include "variant.h"',
            )
            if patched != variant_content:
                print(
                    "[SCONSTRUCT] Patching limboai/compat/variant.cpp for GDExtension compatibility..."
                )
                with open(limboai_variant_cpp, "w") as f:
                    f.write(patched)

    # Patch limboai/compat/variant.h to also activate under ABILITY_SYSTEM_GDEXTENSION
    limboai_variant_h = os.path.join("limboai", "compat", "variant.h")
    if os.path.exists(limboai_variant_h):
        with open(limboai_variant_h, "r") as f:
            variant_h_content = f.read()
        if "ABILITY_SYSTEM_GDEXTENSION" not in variant_h_content:
            patched_h = variant_h_content.replace(
                "#ifdef LIMBOAI_GDEXTENSION\n",
                "#if defined(LIMBOAI_GDEXTENSION) || defined(ABILITY_SYSTEM_GDEXTENSION)\n",
                1,  # Only first occurrence (the include/using block)
            )
            if patched_h != variant_h_content:
                print(
                    "[SCONSTRUCT] Patching limboai/compat/variant.h for GDExtension compatibility..."
                )
                with open(limboai_variant_h, "w") as f:
                    f.write(patched_h)

    # Generate LimboAI version header (required for compilation)
    sys.path.append("limboai")
    try:
        import limboai_version

        # We need to be in the limboai directory for the script to find its 'util' folder
        curr_dir = os.getcwd()
        os.chdir("limboai")
        try:
            limboai_version.generate_module_version_header()
            print("[SCONSTRUCT] Generated LimboAI version header.")
        finally:
            os.chdir(curr_dir)
    except Exception as e:
        print(f"[SCONSTRUCT] Warning: Could not generate limboai_version.gen.h: {e}")

# Configure object folder mapping (VariantDir)
env.VariantDir("src/bin", "src", duplicate=0)

# Collect all source files
sources = Glob("src/bin/*.cpp")
sources += Glob("src/bin/core/*.cpp")
sources += Glob("src/bin/resources/*.cpp")
sources += Glob("src/bin/scene/*.cpp")
sources += Glob("src/bin/editor/*.cpp")
sources += Glob("src/bin/compat/*.cpp")
sources += Glob("src/bin/bridge/*.cpp")

tests_arg = ARGUMENTS.get("tests", env.get("tests", "no"))
if tests_arg in ["unit", "yes"]:
    env.VariantDir("src/bin/tests", "src/tests", duplicate=0)
    sources += Glob("src/bin/tests/*.cpp")

# Collect LimboAI sources if available
if os.path.isdir("limboai"):
    env.VariantDir("src/bin/limboai", "limboai", duplicate=0)
    sources += Glob("src/bin/limboai/*.cpp")
    sources += Glob("src/bin/limboai/blackboard/*.cpp")
    sources += Glob("src/bin/limboai/blackboard/bb_param/*.cpp")
    sources += Glob("src/bin/limboai/bt/*.cpp")
    sources += Glob("src/bin/limboai/bt/tasks/*.cpp")
    sources += Glob("src/bin/limboai/bt/tasks/blackboard/*.cpp")
    sources += Glob("src/bin/limboai/bt/tasks/composites/*.cpp")
    sources += Glob("src/bin/limboai/bt/tasks/decorators/*.cpp")
    sources += Glob("src/bin/limboai/bt/tasks/misc/*.cpp")
    sources += Glob("src/bin/limboai/bt/tasks/scene/*.cpp")
    sources += Glob("src/bin/limboai/bt/tasks/utility/*.cpp")
    sources += Glob("src/bin/limboai/compat/*.cpp")
    sources += Glob("src/bin/limboai/hsm/*.cpp")
    sources += Glob("src/bin/limboai/util/*.cpp")
    sources += Glob("src/bin/limboai/editor/debugger/*.cpp")
    sources += Glob("src/bin/limboai/editor/*.cpp")

if env["target"] in ["editor"]:
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
