#!/usr/bin/env python
import os
import sys
import subprocess
import shutil


def run_tests():
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    demo_dir = os.path.join(project_root, "demo")

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
            print(
                "[ABILITY SYSTEM] Godot not found in CI Build Step. Skipping tests (will run in dedicated test step)."
            )
            return 0

        print(f"Error: Godot binary '{godot_bin}' not found.")
        print("Please add Godot to your PATH or set GODOT_BIN environment variable.")
        return 1

    print("\n[ABILITY SYSTEM] Running Tests via Godot Headless...")
    print(f"Godot: {godot_bin}")
    print(f"Path: {demo_dir}\n")

    # Pre-run: Godot --editor to import assets and extensions
    import_cmd = [
        godot_bin,
        "--headless",
        "--editor",
        "--quit",
        "--path",
        demo_dir,
        "--log-file",
        "../import.log",
    ]

    # Parse test type from arguments if available
    test_type = "unit"
    if len(sys.argv) > 1:
        test_type = sys.argv[1].lower()

    if test_type == "integration":
        if is_ci:
            # Command: godot --headless --path demo --run-ability-tests-integration --quit
            cmd = [
                godot_bin,
                "--headless",
                "--quit",
                "--path",
                demo_dir,
                "--run-ability-tests-integration",
            ]
        else:
            print(
                "[ABILITY SYSTEM] Integration UI Tests must NOT be run headlessly from SConstruct. These are manual interactive tests."
            )
            return 0
    elif test_type == "demo":
        # Command: godot --headless --path demo --quit
        cmd = [
            godot_bin,
            "--headless",
            "--quit",
            "--path",
            demo_dir,
        ]
    else:
        # Command: godot --headless --path demo --run-ability-tests-unit --quit
        cmd = [
            godot_bin,
            "--headless",
            "--quit",
            "--path",
            demo_dir,
            "--run-ability-tests-unit",
        ]

    try:
        # Create a copy of environment and remove any variable with non-ASCII characters
        # GitHub Actions sometimes puts emojis in GITHUB_WORKFLOW, which crashes subprocess on Windows
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

        print("[ABILITY SYSTEM] Triggering import pass (--editor)...")
        subprocess.run(
            import_cmd, capture_output=True, text=True, timeout=120, env=clean_env
        )

        print(f"[ABILITY SYSTEM] Running {test_type} tests...")

        log_path = os.path.abspath(
            os.path.join(project_root, f"doctest_{test_type}.log")
        )

        # Open log file for writing
        with open(log_path, "w", encoding="utf-8", errors="replace") as log_file:
            # Run Godot and stream output
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
            try:
                # Read output line by line in real-time
                while True:
                    line = process.stdout.readline()
                    if not line and process.poll() is not None:
                        break
                    if line:
                        print(line, end="")  # Terminal
                        log_file.write(line)  # Log file
                        log_file.flush()
                        full_output.append(line)

                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                print("\n[TIMEOUT] Process hang detected after completion. Killing...")
                process.kill()

            combined_stdout = "".join(full_output)
            test_success = "[doctest] Status: SUCCESS!" in combined_stdout
            returncode = process.returncode

        if test_type == "demo":
            if returncode == 0:
                print("\n[SUCCESS] DEMO RUN COMPLETED")
                return 0
            else:
                print(f"\n[FAILED] DEMO RUN FAILED (Exit Code: {returncode})")
                return returncode
        else:
            if test_success:
                print(f"\n[SUCCESS] {test_type.upper()} TEST RUN COMPLETED")
                return 0
            else:
                print(
                    f"\n[FAILED] {test_type.upper()} TEST RUN FAILED OR DID NOT REPORT SUCCESS (Exit Code: {returncode})"
                )
                return returncode if returncode != 0 else 1

    except subprocess.TimeoutExpired:
        print("\n[TIMEOUT] TEST RUN TIMED OUT (Maximum 120 seconds reached)")
        return 1
    except Exception as e:
        print(f"Failed to execute Godot: {repr(e)}")
        return 1


if __name__ == "__main__":
    sys.exit(run_tests())
