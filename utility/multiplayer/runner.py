#!/usr/bin/env python3
"""
Multiplayer Test Runner with Latency Simulation & Chaos Engine
Orchestrates 10 players with configurable network stress
"""

import os
import sys
import subprocess
import argparse
import time
import random
import threading
from pathlib import Path
from typing import Optional


class MultiplayerTestRunner:
    """
    Advanced multiplayer test runner with:
    - Simulated network latency (50-200ms configurable)
    - Chaos engine (kill/restart clients)
    - State hash validation
    - Detailed logging & reporting
    """

    def __init__(
        self,
        player_id: int,
        server_port: int = 7777,
        server_host: str = "127.0.0.1",
        latency_min_ms: int = 50,
        latency_max_ms: int = 200,
        chaos_enabled: bool = False,
        chaos_probability: float = 0.05,
    ):
        self.player_id = player_id
        self.server_port = server_port
        self.server_host = server_host
        self.latency_min_ms = latency_min_ms
        self.latency_max_ms = latency_max_ms
        self.chaos_enabled = chaos_enabled
        self.chaos_probability = chaos_probability

        self.log_dir = Path("utility/multiplayer/logs")
        self.log_dir.mkdir(parents=True, exist_ok=True)

        self.log_file = self.log_dir / f"doctest_mp_player_{player_id}.log"
        self.process: Optional[subprocess.Popen] = None  # type: ignore
        self.chaos_thread: Optional[threading.Thread] = None

    def get_demo_binary(self) -> str:
        """Find compiled demo executable or fallback to Godot"""
        demo_bin = Path("bin/demo/demo.x86_64")
        if demo_bin.exists():
            return str(demo_bin)

        godot_bin = os.environ.get("GODOT_BIN", "godot")
        return godot_bin

    def get_command(self) -> list:
        """Build command to launch player"""
        demo_bin = self.get_demo_binary()

        # Check if it's a compiled executable or Godot project
        if demo_bin.endswith(".x86_64") and Path(demo_bin).exists():
            # Compiled executable mode
            return [demo_bin]
        else:
            # Godot project mode
            return [
                demo_bin,
                "--headless",
                "--path",
                "demo",
                "-s",
                "mp_test_runner.gd",
            ]

    def get_environment(self) -> dict:
        """Setup environment with latency simulation"""
        env = os.environ.copy()
        env["MP_GHA_PLAYER_ID"] = str(self.player_id)
        env["MP_SERVER_PORT"] = str(self.server_port)
        env["MP_SERVER_HOST"] = self.server_host

        # Add simulated latency if not server
        if self.player_id > 0:
            latency_ms = random.randint(self.latency_min_ms, self.latency_max_ms)
            env["MP_SIMULATED_LATENCY_MS"] = str(latency_ms)
            print(f"[MP-RUNNER] Player {self.player_id}: Latency {latency_ms}ms")

        return env

    def launch(self) -> bool:
        """Launch player process"""
        print(f"[MP-RUNNER] Starting Player {self.player_id}...")

        cmd = self.get_command()
        env = self.get_environment()

        print(f"[MP-RUNNER] Command: {' '.join(cmd)}")

        try:
            with open(self.log_file, "w") as f:
                self.process = subprocess.Popen(
                    cmd,
                    env=env,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                    bufsize=1,
                )

                # Stream output to both console and file
                if self.process.stdout:
                    for line in self.process.stdout:
                        sys.stdout.write(f"[Player {self.player_id}] {line}")
                        f.write(line)

                try:
                    self.process.wait(timeout=300)  # 5 minutes
                except subprocess.TimeoutExpired:
                    print(
                        f"[MP-RUNNER] Player {self.player_id} timeout (exceeded 300s)"
                    )
                    self.process.kill()
                    f.write("\n[TIMEOUT] Test exceeded 300 seconds\n")
                    return False

                if self.process.returncode != 0:
                    print(
                        f"[MP-RUNNER] Player {self.player_id} finished with code {self.process.returncode}"
                    )
                    return False
                else:
                    print(f"[MP-RUNNER] Player {self.player_id} finished successfully")
                    return True

        except Exception as e:
            print(f"[MP-RUNNER] Critical error: {e}")
            return False

    def start_chaos_engine(self) -> None:
        """Start background chaos thread (kills/restarts player randomly)"""
        if not self.chaos_enabled or self.player_id == 0:
            return

        print(f"[CHAOS] Chaos engine enabled for player {self.player_id}")

        self.chaos_thread = threading.Thread(target=self._chaos_loop, daemon=True)
        self.chaos_thread.start()

    def _chaos_loop(self) -> None:
        """Background loop that randomly kills/restarts client"""
        restart_count = 0

        while self.process and self.process.poll() is None:
            time.sleep(1)

            # Random kill with configured probability
            if random.random() < self.chaos_probability:
                print(f"[CHAOS] Killing player {self.player_id}...")
                self.process.kill()

                # Restart after random delay
                restart_delay = random.randint(3, 8)
                print(
                    f"[CHAOS] Restarting player {self.player_id} in {restart_delay}s..."
                )
                time.sleep(restart_delay)

                # Relaunch
                restart_count += 1
                print(
                    f"[CHAOS] Relaunching player {self.player_id} (attempt {restart_count})..."
                )
                self.launch()


def main():
    parser = argparse.ArgumentParser(
        description="Multiplayer Test Runner for Ability System"
    )
    parser.add_argument(
        "--player-id",
        type=int,
        required=True,
        help="Player ID (0=server, 1-10=clients)",
    )
    parser.add_argument(
        "--server-port", type=int, default=7777, help="Server port (default 7777)"
    )
    parser.add_argument(
        "--server-host",
        type=str,
        default="127.0.0.1",
        help="Server host (default 127.0.0.1)",
    )
    parser.add_argument(
        "--latency-min",
        type=int,
        default=50,
        help="Minimum simulated latency in ms (default 50)",
    )
    parser.add_argument(
        "--latency-max",
        type=int,
        default=200,
        help="Maximum simulated latency in ms (default 200)",
    )
    parser.add_argument(
        "--chaos", action="store_true", help="Enable chaos engine (random kill/restart)"
    )
    parser.add_argument(
        "--chaos-probability",
        type=float,
        default=0.05,
        help="Chaos kill probability per second (default 0.05 = 5%)",
    )

    args = parser.parse_args()

    runner = MultiplayerTestRunner(
        player_id=args.player_id,
        server_port=args.server_port,
        server_host=args.server_host,
        latency_min_ms=args.latency_min,
        latency_max_ms=args.latency_max,
        chaos_enabled=args.chaos,
        chaos_probability=args.chaos_probability,
    )

    # Start chaos engine if enabled
    runner.start_chaos_engine()

    # Launch and wait
    success = runner.launch()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
