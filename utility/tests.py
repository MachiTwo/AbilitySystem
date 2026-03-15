#!/usr/bin/env python
import os
import sys
import subprocess
import shutil


def run_godot_command(godot_bin, project_dir, args, test_type, clean_env, project_root):
    """Utility to run a Godot command and capture its output."""
    import_cmd = [
        godot_bin,
        "--headless",
        "--editor",
        "--quit",
        "--path",
        project_dir,
        "--log-file",
        os.path.join(project_root, f"import_{test_type}.log"),
    ]

    print(f"[ABILITY SYSTEM] Triggering import pass for {test_type} (--editor)...")
    try:
        subprocess.run(
            import_cmd, capture_output=True, text=True, timeout=120, env=clean_env
        )
    except subprocess.TimeoutExpired:
        print(f"\n[TIMEOUT] Import pass for {test_type} timed out.")
        return 1

    print(f"[ABILITY SYSTEM] Running {test_type} tests...")

    cmd = [godot_bin, "--headless", "--path", project_dir] + args
    if test_type != "playtest":
        cmd.append("--quit")

    log_path = os.path.abspath(os.path.join(project_root, f"doctest_{test_type}.log"))

    with open(log_path, "w", encoding="utf-8", errors="replace") as log_file:
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            env=clean_env,
            encoding="utf-8",
            errors="replace",
        )

        full_output = []
        import time

        start_time = time.time()
        timeout_seconds = 600  # 10 minutes total max for any test step

        try:
            while True:
                if time.time() - start_time > timeout_seconds:
                    raise subprocess.TimeoutExpired(cmd, timeout_seconds)

                line = process.stdout.readline()
                if not line and process.poll() is not None:
                    break
                if line:
                    print(line, end="")
                    log_file.write(line)
                    log_file.flush()
                    full_output.append(line)

            process.wait(timeout=30)  # Increased wait to ensure process finishes
        except subprocess.TimeoutExpired:
            print(
                f"\n[TIMEOUT] Process hang detected for {test_type} (Max 10m). Killing..."
            )
            process.kill()
            process.wait()  # Ensure it's gone

        combined_stdout = "".join(full_output)
        test_success = (
            "[doctest] Status: SUCCESS!" in combined_stdout
            or "[SUCCESS]" in combined_stdout
        )
        returncode = process.returncode

    if test_type in ["demo", "playtest"]:
        if returncode == 0:
            print(f"\n[SUCCESS] {test_type.upper()} RUN COMPLETED")
            return 0
        else:
            print(
                f"\n[FAILED] {test_type.upper()} RUN FAILED (Exit Code: {returncode})"
            )
            return returncode if returncode != 0 else 1
    else:
        if test_success:
            print(f"\n[SUCCESS] {test_type.upper()} TEST RUN COMPLETED")
            return 0
        else:
            print(
                f"\n[FAILED] {test_type.upper()} TEST RUN FAILED OR DID NOT REPORT SUCCESS (Exit Code: {returncode})"
            )
            return returncode if returncode != 0 else 1


def run_tests():
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    # Parse test type from arguments if available
    test_type = "unit"
    if len(sys.argv) > 1:
        test_type = sys.argv[1].lower()

    # Try to find Godot binary
    godot_bin = os.environ.get("GODOT_BIN", "godot")
    if not shutil.which(godot_bin):
        # Common names and user specific path
        search_list = [
            "C:/Users/bruno/Desktop/Godot_v4.6.1-stable_win64.exe",
            "Godot_v4.6.1-stable_win64.exe",
            "Godot_v4.6-stable_win64.exe",
            "godot.exe",
        ]
        for name in search_list:
            if shutil.which(name) or os.path.exists(name):
                godot_bin = name
                break

    is_ci = os.environ.get("GITHUB_ACTIONS") == "true"
    must_find = os.environ.get("GODOT_BIN") is not None

    if not shutil.which(godot_bin):
        if is_ci and not must_find:
            print("[ABILITY SYSTEM] Godot not found in CI Build Step. Skipping tests.")
            return 0

        print(f"Error: Godot binary '{godot_bin}' not found.")
        return 1

    print("\n[ABILITY SYSTEM] Running Tests via Godot Headless...")
    print(f"Godot: {godot_bin}")

    # Prepare environment (handling non-ascii characters in Windows env)
    clean_env = os.environ.copy()
    keys_to_remove = []
    for key, value in clean_env.items():
        try:
            key.encode("ascii")
            value.encode("ascii")
        except UnicodeEncodeError:
            keys_to_remove.append(key)

    for key in keys_to_remove:
        del clean_env[key]

    # Define execution plans
    plans = []
    if test_type == "all":
        plans.append(("unit", "demo", ["--run-ability-tests-unit"]))
        plans.append(
            (
                "playtest",
                "demo-integration",
                ["res://scenes/showcase.tscn", "--playtest-cycle"],
            )
        )
    elif test_type == "playtest":
        plans.append(
            (
                "playtest",
                "demo-integration",
                ["res://scenes/showcase.tscn", "--playtest-cycle"],
            )
        )
    elif test_type == "unit":
        plans.append(("unit", "demo", ["--run-ability-tests-unit"]))
    elif test_type == "integration":
        if is_ci:
            plans.append(("integration", "demo", ["--run-ability-tests-integration"]))
        else:
            print(
                "[ABILITY SYSTEM] Integration UI Tests must NOT be run headlessly from SConstruct."
            )
            return 0
    elif test_type == "demo":
        plans.append(("demo", "demo", []))
    else:
        # Fallback to single run
        project_folder = "demo-integration" if test_type == "playtest" else "demo"
        plans.append((test_type, project_folder, ["--run-ability-tests-unit"]))

    # Execute plans
    for t_type, p_folder, args in plans:
        p_dir = os.path.join(project_root, p_folder)
        res = run_godot_command(godot_bin, p_dir, args, t_type, clean_env, project_root)
        if res != 0:
            print(f"\n[ERROR] Step '{t_type}' failed with exit code {res}. Aborting.")
            return res

    return 0


if __name__ == "__main__":
    sys.exit(run_tests())
