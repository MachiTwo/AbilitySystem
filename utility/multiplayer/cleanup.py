import os
import psutil
import time


def cleanup_zombies():
    print("[MP CLEANUP] Scanning for orphaned Godot/Test headless instances...")
    killed = 0
    start_time = time.time()

    # Iterate through all running processes
    for proc in psutil.process_iter(["pid", "name", "cmdline"]):
        try:
            name = proc.info["name"]
            cmdline = proc.info.get("cmdline", [])

            # Check if it's a Godot instance running headless or MP tests
            if name and "godot" in name.lower() or "Godot_v4" in name:
                if cmdline and any(
                    "Test_MP" in arg or "--headless" in arg for arg in cmdline
                ):
                    print(
                        f"  [X] Killing Zombie PID: {proc.info['pid']} -> {' '.join(cmdline[:3])}..."
                    )
                    proc.kill()
                    killed += 1

        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            pass

    print(
        f"[MP CLEANUP] Completed in {time.time() - start_time:.2f}s. Terminated {killed} processes."
    )


if __name__ == "__main__":
    # Needs pip install psutil to work if psutil is not built-in,
    # but GHA python runners have ways or we can fallback to os.system taskkill.

    try:
        cleanup_zombies()
    except ImportError:
        print("[MP CLEANUP] Pre-hook fallback (No Psutil found)")
        # Fallback for Windows
        if os.name == "nt":
            os.system('taskkill /F /IM "godot*" /T >nul 2>&1')
            os.system('taskkill /F /IM "Godot_v4*" /T >nul 2>&1')
        else:  # Fallback Linux
            os.system('pkill -f "godot.*--headless" >/dev/null 2>&1')
        print("[MP CLEANUP] Fallback raw kill sent.")
