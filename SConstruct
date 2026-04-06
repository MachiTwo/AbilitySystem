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
opts.Add("tests", "Tests to run (no, unit, bridge, yes)", "no")
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
        with open(gcpp_tool_path, "w") as f:
            f.write(new_content)

gcpp_sconstruct_path = "godot-cpp/SConstruct"
if os.path.exists(gcpp_sconstruct_path):
    with open(gcpp_sconstruct_path, "r") as f:
        content = f.read()
    if 'Decider("MD5")' not in content:
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

# --- Godot-CCP Build (Isolated from AS defines) ---
gcpp_env = env.Clone()
env = SConscript("godot-cpp/SConstruct", {"env": gcpp_env, "customs": customs})

# Now add AS-specific defines to the returned environment
env.Append(CPPPATH=[".", "src"])
env.Append(CPPDEFINES=["ABILITY_SYSTEM_GDEXTENSION"])

# --- Object Directory and Define Selection ---
tests_arg = ARGUMENTS.get("tests", env.get("tests", "no"))
bin_prefix = "bin"

if tests_arg != "no":
    bin_prefix = f"bin/test-{tests_arg}"
    env.Append(CPPDEFINES=["AS_TESTS_ENABLED"])
    if tests_arg == "unit":
        env.Append(CPPDEFINES=["AS_UNIT_TESTS_ENABLED"])
    elif tests_arg == "bridge":
        env.Append(CPPDEFINES=["AS_BRIDGE_TESTS_ENABLED"])
    elif tests_arg == "yes":
        env.Append(CPPDEFINES=["AS_UNIT_TESTS_ENABLED", "AS_BRIDGE_TESTS_ENABLED"])

# Configure object folder mapping (VariantDir)
env.VariantDir(f"src/{bin_prefix}", "src", duplicate=0)

# Collect all source files using the specific bin prefix
sources = []
sources += Glob(f"src/{bin_prefix}/*.cpp")
sources += Glob(f"src/{bin_prefix}/core/*.cpp")
sources += Glob(f"src/{bin_prefix}/resources/*.cpp")
sources += Glob(f"src/{bin_prefix}/scene/*.cpp")
sources += Glob(f"src/{bin_prefix}/editor/*.cpp")
sources += Glob(f"src/{bin_prefix}/compat/*.cpp")
sources += Glob(f"src/{bin_prefix}/bridge/*.cpp")

if tests_arg != "no":
    sources += Glob(f"src/{bin_prefix}/tests/*.cpp")

# --- LimboAI Integration ---
if os.path.isdir("src/limboai"):
    # Integrated sources into AS binary using the dynamic bin prefix
    env.VariantDir(f"src/{bin_prefix}/limboai", "src/limboai", duplicate=0)
    limbo_sources = Glob(f"src/{bin_prefix}/limboai/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/blackboard/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/blackboard/bb_param/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/bt/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/bt/tasks/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/bt/tasks/blackboard/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/bt/tasks/composites/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/bt/tasks/decorators/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/bt/tasks/misc/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/bt/tasks/scene/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/bt/tasks/utility/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/compat/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/hsm/*.cpp")
    if env["target"] != "template_release":
        limbo_sources += Glob(f"src/{bin_prefix}/limboai/editor/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/editor/debugger/*.cpp")
    limbo_sources += Glob(f"src/{bin_prefix}/limboai/util/*.cpp")

    sources += limbo_sources
    env.Append(CPPDEFINES=["LIMBOAI_SUBMODULE_BUILD"])
    env.Append(CPPPATH=["src/limboai"])
    env.Append(CPPDEFINES=["LIMBOAI_GDEXTENSION"])
    env.Append(CPPDEFINES=[("limboai_init", "limboai_init_unused")])

    # --- LimboAI Automatic Patching (CI Resilience) ---
    limboai_variant_cpp = os.path.join("src", "limboai", "compat", "variant.cpp")
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
                with open(limboai_variant_cpp, "w") as f:
                    f.write(patched)

    limboai_variant_h = os.path.join("src", "limboai", "compat", "variant.h")
    if os.path.exists(limboai_variant_h):
        with open(limboai_variant_h, "r") as f:
            variant_h_content = f.read()
        if "ABILITY_SYSTEM_GDEXTENSION" not in variant_h_content:
            patched_h = variant_h_content.replace(
                "#ifdef LIMBOAI_GDEXTENSION\n",
                "#if defined(LIMBOAI_GDEXTENSION) || defined(ABILITY_SYSTEM_GDEXTENSION)\n",
                1,
            )
            if patched_h != variant_h_content:
                with open(limboai_variant_h, "w") as f:
                    f.write(patched_h)

    # Generate LimboAI version header
    sys.path.append("src/limboai")
    try:
        import limboai_version

        curr_dir = os.getcwd()
        os.chdir("src/limboai")
        try:
            limboai_version.generate_module_version_header()
        finally:
            os.chdir(curr_dir)
    except Exception as e:
        print(f"[SCONSTRUCT] Warning: Could not generate limboai_version.gen.h: {e}")


if env["target"] in ["editor"]:
    try:
        doc_data = env.GodotCPPDocData(
            f"src/{bin_prefix}/gen/doc_data.gen.cpp",
            source=Glob("src/doc_classes/*.xml"),
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
