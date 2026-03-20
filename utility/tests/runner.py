#!/usr/bin/env python
import os
import sys
import subprocess
import argparse
from pathlib import Path

class TestRunner:
    """
    Simulation Runner for AbilitySystem.
    Handles high-level simulation tests: singleplayer and multiplayer.
    Execution is headless-by-default.
    Note: Unit and Integration tests are handled directly by the build system/Godot.
    """

    def __init__(self, godot_path=None, headless=True):
        self.godot_path = godot_path or os.environ.get("GODOT_BIN", "godot")
        self.headless = headless
        # Root directory is 3 levels up from utility/tests/runner.py
        self.base_dir = Path(__file__).resolve().parent.parent.parent
        self.project_dir = self.base_dir / "demo"

    def _get_common_args(self, include_headless=True):
        args = []
        if self.headless and include_headless:
            args += ["--headless"]
        
        args += ["--audio-driver", "Dummy"]
        
        if os.name != 'nt' and self.headless:
            args += ["--display-driver", "opengl3"]
            
        return args

    def run_command(self, extra_args, quit_after=True, project_path=None):
        cmd = [self.godot_path]
        cmd += self._get_common_args()
        
        if project_path:
            cmd += ["--path", str(project_path.absolute())]
        
        if quit_after:
            cmd += ["--quit"]
        
        cmd += extra_args
        
        print(f"[Runner] Executing Simulation: {' '.join(cmd)}")
        
        try:
            process = subprocess.Popen(
                cmd, 
                stdout=subprocess.PIPE, 
                stderr=subprocess.STDOUT, 
                text=True,
                encoding="utf-8",
                errors="replace",
                bufsize=1
            )
            
            while True:
                line = process.stdout.readline()
                if not line and process.poll() is not None:
                    break
                if line:
                    sys.stdout.write(line)
                    sys.stdout.flush()
            
            process.wait()
            return process.returncode
        except Exception as e:
            print(f"[Runner] Error: {e}")
            return 1

    def run_singleplayer_test(self, scene=None):
        scene_name = scene or "main.tscn"
        print(f"\n[Runner] >>> Running Singleplayer Simulation (Scene: {scene_name}) <<<")
        args = [f"res://{scene_name}"]
        return self.run_command(args, project_path=self.project_dir, quit_after=self.headless)

    def run_multiplayer_tests(self, num_clients=2):
        print(f"\n[Runner] >>> Running Multiplayer Simulation ({num_clients} clients) <<<")
        # Multiplayer logic is pending Phase 4 implementation
        # Typically involves spawning a server and multiple clients
        print("[Runner] Multi-instance spawning is pending Phase 4.")
        return 0

def main():
    parser = argparse.ArgumentParser(description="AbilitySystem Simulation Runner")
    parser.add_argument(
        "--test", 
        type=str, 
        choices=["singleplayer", "multiplayer"], 
        required=True,
        help="Simulation mode to execute"
    )
    parser.add_argument(
        "--godot", 
        type=str, 
        help="Path to Godot bin"
    )
    parser.add_argument(
        "--no-headless", 
        action="store_true", 
        help="Disable headless mode"
    )
    parser.add_argument(
        "--clients", 
        type=int, 
        default=2, 
        help="Number of clients"
    )
    parser.add_argument(
        "--scene",
        type=str,
        help="Target scene"
    )
    
    args = parser.parse_args()
    
    runner = TestRunner(godot_path=args.godot, headless=not args.no_headless)
    
    results = {}
    
    if args.test == "singleplayer":
        results["singleplayer"] = runner.run_singleplayer_test(scene=args.scene)
    elif args.test == "multiplayer":
        results["multiplayer"] = runner.run_multiplayer_tests(num_clients=args.clients)
        
    failed = False
    for test, code in results.items():
        if code != 0:
            failed = True
            print(f"[Runner] {test.upper()} FAILED with code {code}")
    
    if failed:
        sys.exit(1)
    else:
        sys.exit(0)

if __name__ == "__main__":
    main()
