#!/usr/bin/env python3
import os
import sys
import subprocess
import argparse
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(
        description="Multiplayer Test Runner for Ability System"
    )
    parser.add_argument("--player-id", type=int, required=True, help="Player ID (1-10)")
    parser.add_argument("--server-port", type=int, default=7777, help="Server port")
    args = parser.parse_args()

    # Configuration
    godot_bin = os.environ.get("GODOT_BIN", "godot")
    project_path = "demo"
    log_dir = Path("utility/multiplayer/logs")
    log_dir.mkdir(parents=True, exist_ok=True)

    log_file = log_dir / f"doctest_mp_player_{args.player_id}.log"

    print(f"[MP-RUNNER] Starting Player {args.player_id}...")

    # Environment variables for Godot
    env = os.environ.copy()
    env["MP_GHA_PLAYER_ID"] = str(args.player_id)
    env["MP_SERVER_PORT"] = str(args.server_port)

    # Run Godot command
    # - --headless: No window
    # - --path: Path to project
    # - -s: Script to run (test runner)
    cmd = [godot_bin, "--headless", "--path", project_path, "-s", "mp_test_runner.gd"]

    print(f"[MP-RUNNER] Command: {' '.join(cmd)}")

    try:
        with open(log_file, "w") as f:
            # We redirect stdout/stderr to the log file for analysis
            process = subprocess.Popen(
                cmd,
                env=env,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
            )

            # Read line by line and print to both console and file
            for line in process.stdout:
                sys.stdout.write(f"[Player {args.player_id}] {line}")
                f.write(line)

            process.wait()

            if process.returncode != 0:
                print(
                    f"[MP-RUNNER] Player {args.player_id} finished with error code {process.returncode}"
                )
                # We don't sys.exit here to allow the log analyzer to catch the summary
            else:
                print(f"[MP-RUNNER] Player {args.player_id} finished successfully.")

    except Exception as e:
        print(f"[MP-RUNNER] Critical failure: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
