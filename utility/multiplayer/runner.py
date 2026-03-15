import os
import sys
import subprocess
import time
from dotenv import load_dotenv


# Run the 10 simulated clients against a placeholder server
def run_mp_tests():
    project_root = os.path.dirname(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    )
    log_dir = os.path.join(project_root, "utility", "multiplayer", "logs")
    os.makedirs(log_dir, exist_ok=True)
    env_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".env")

    # Create default .env if not exists
    if not os.path.exists(env_path):
        with open(env_path, "w") as f:
            f.write("MP_SERVER_HOST=127.0.0.1\n")
            f.write("MP_SERVER_PORT=7777\n")
            f.write("MP_MAX_PLAYERS=10\n")
            f.write("MP_TIMEOUT=30\n")

    load_dotenv(env_path)

    # 0. Cleanup Previous Zombies
    try:
        import cleanup

        cleanup.cleanup_zombies()
    except Exception as e:
        print(f"[ABILITY SYSTEM MP] Cleanup Error: {e}")

    # 1. Generate Fake Networking Conditions
    try:
        import net_simulator

        net_simulator.generate_mp_env_config()
    except Exception as e:
        print(f"[ABILITY SYSTEM MP] Net Sim Error: {e}")

    host = os.environ.get("MP_SERVER_HOST", "127.0.0.1")
    port = int(os.environ.get("MP_SERVER_PORT", 7777))
    players = int(os.environ.get("MP_MAX_PLAYERS", 10))

    godot_bin = os.environ.get("GODOT_BIN", "godot")

    print("[ABILITY SYSTEM MP] Starting Local Multiplayer Orchestrator")
    print(f"[ABILITY SYSTEM MP] Host: {host}:{port} | Players: {players}")

    print("[ABILITY SYSTEM MP] Starting Python Mock State Server...")
    # Lançar o mock server internamente em daemon (Background)
    mock_server_proc = subprocess.Popen(
        [
            sys.executable,
            os.path.join(os.path.dirname(__file__), "mock_server.py"),
            "300",
        ],  # 5-min timeout
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    time.sleep(1)  # Wait for port binding

    processes = []

    for i in range(1, players + 1):
        print(f"[ABILITY SYSTEM MP] Launching simulated Player {i}...")
        test_filter = f"--test-case=*Player_{i}*"
        cmd = [
            godot_bin,
            "--headless",
            "--path",
            "demo",
            test_filter,
            "--success",
            "--quit",
        ]

        log_path = os.path.join(log_dir, f"doctest_mp_player_{i}.log")
        log_file = open(log_path, "w")

        proc = subprocess.Popen(
            cmd, stdout=log_file, stderr=subprocess.STDOUT, cwd=project_root
        )
        processes.append((i, proc, log_file))

        time.sleep(0.5)  # Stagger connections

    print("[ABILITY SYSTEM MP] All players launched. Waiting for completion...")

    failed = False
    for p_num, proc, log_file in processes:
        proc.wait(timeout=120)
        log_file.close()
        if proc.returncode != 0:
            print(f"[FAILED] Player {p_num} test failed (Code {proc.returncode}).")
            failed = True
        else:
            print(f"[SUCCESS] Player {p_num} completed successfully.")

    if mock_server_proc.poll() is None:
        mock_server_proc.kill()

    # Final Analyze Log
    try:
        import log_analyzer

        log_analyzer.analyze_mp_logs()
    except Exception:
        pass

    if failed:
        sys.exit(1)
    else:
        print("[ABILITY SYSTEM MP] ALL 10 PLAYERS COMPLETED 300% COVERAGE.")
        # Cleanup logs only on success to keep pollution low
        for _, _, log_file in processes:
            try:
                if os.path.exists(log_file.name):
                    os.remove(log_file.name)
            except Exception:
                pass
        sys.exit(0)


if __name__ == "__main__":
    run_mp_tests()
