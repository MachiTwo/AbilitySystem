#!/usr/bin/env python
import os
import sys

from methods import print_error


libname = "ability_system"
projectdir = "demo"

build_tests = False
test_type = "unit"

if "tests" in ARGUMENTS:
    test_arg = ARGUMENTS["tests"].lower()
    if test_arg in ("yes", "1", "true", "on", "unit"):
        build_tests = True
        test_type = "unit"
    elif test_arg in ("integration", "all"):
        build_tests = True
        test_type = test_arg
    del ARGUMENTS["tests"]

import SCons.Script

SCons.Script.ARGLIST = [x for x in SCons.Script.ARGLIST if x[0] != "tests"]

localEnv = Environment(tools=["default"], PLATFORM="")

# Build profiles can be used to decrease compile times.
# You can either specify "disabled_classes", OR
# explicitly specify "enabled_classes" which disables all other classes.
# Modify the example file as needed and uncomment the line below or
# manually specify the build_profile parameter when running SCons.

# localEnv["build_profile"] = "build_profile.json"

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()

if not (os.path.isdir("godot-cpp") and os.listdir("godot-cpp")):
    print_error("""godot-cpp is not available within this folder, as Git submodules haven't been initialized.
Run the following command to download godot-cpp:

    git submodule update --init --recursive""")
    sys.exit(1)

env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

env.Append(CPPPATH=[".", "src", "src/tests"])
env.Append(CPPDEFINES=["ABILITY_SYSTEM_GDEXTENSION"])

# Configure object folder mapping (VariantDir)
env.VariantDir("src/bin", "src", duplicate=0)

# Collect all source files from src/bin/ mapping and its subdirectories.
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
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

# --- Test Build Logic ---
if build_tests:
    env.Append(CPPDEFINES=["ABILITY_SYSTEM_TESTS_ENABLED"])
    # Include tests in the main library build
    env.VariantDir("src/bin/tests", "src/tests", duplicate=0)
    sources += Glob("src/bin/tests/*.cpp")

# --- Target Definition ---
# .dev doesn't inhibit compatibility, so we don't need to key it.
# .universal just means "compatible with all relevant arches" so we don't need to key it.
suffix = env["suffix"].replace(".dev", "").replace(".universal", "")

lib_filename = "{}{}{}{}".format(
    env.subst("$SHLIBPREFIX"), libname, suffix, env.subst("$SHLIBSUFFIX")
)

library = env.SharedLibrary(
    "addons/ability_system/bin/{}".format(lib_filename),
    source=sources,
)

# --- Post-Build / Sync / Test ---
# Sync action (post_build.py)
post_build_script = os.path.join("utility", "post_build.py")
post_build_action = None
if os.path.exists(post_build_script):
    post_build_action = env.Command(
        "post_build_dummy", library, f"{sys.executable} {post_build_script}"
    )
    env.AlwaysBuild(post_build_action)
    env.Depends(post_build_action, library)

if build_tests:
    # After building the library AND syncing it, run the test utility
    is_ci = os.environ.get("GITHUB_ACTIONS") == "true"
    if is_ci:
        test_run = env.Alias("run_tests_dummy", library)
    else:
        test_run = env.Command(
            "run_tests_dummy",
            library,
            f"{sys.executable} utility/tests.py {test_type}",
        )
    env.AlwaysBuild(test_run)
    if post_build_action:
        env.Depends(test_run, post_build_action)
    else:
        env.Depends(test_run, library)

    Default(test_run)
else:
    if post_build_action:
        Default(library, post_build_action)
    else:
        Default(library)
