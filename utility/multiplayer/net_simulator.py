import json
import os


def generate_mp_env_config():
    """Creates a JSON file containing simulated latency profiles for each agent.
    The C++ side can read this if Godot's built-in FileAccess is used during tests
    to mock get_ping() or packet delays."""

    config_path = os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "mp_simulation_profile.json"
    )

    # 300% variation profile for network conditions
    profiles = {}

    # 1. Perfect Connection
    profiles["Player_1"] = {"latency_ms": 10, "packet_loss_pct": 0.0, "jitter_ms": 2}
    profiles["Player_2"] = {"latency_ms": 15, "packet_loss_pct": 0.0, "jitter_ms": 5}
    profiles["Player_3"] = {"latency_ms": 20, "packet_loss_pct": 0.0, "jitter_ms": 3}

    # 2. Average/Laggy
    profiles["Player_4"] = {"latency_ms": 80, "packet_loss_pct": 1.5, "jitter_ms": 15}
    profiles["Player_5"] = {"latency_ms": 120, "packet_loss_pct": 3.0, "jitter_ms": 25}
    profiles["Player_6"] = {"latency_ms": 180, "packet_loss_pct": 5.0, "jitter_ms": 40}

    # 3. Hostile/Disconnecting Conditions
    profiles["Player_7"] = {
        "latency_ms": 350,
        "packet_loss_pct": 15.0,
        "jitter_ms": 100,
    }
    profiles["Player_8"] = {
        "latency_ms": 500,
        "packet_loss_pct": 30.0,
        "jitter_ms": 200,
    }
    profiles["Player_9"] = {
        "latency_ms": 999,
        "packet_loss_pct": 80.0,
        "jitter_ms": 500,
    }

    # Hack/Cheat Simulation Profile (The API should ban this automatically in future tests)
    profiles["Player_10"] = {
        "latency_ms": 0,
        "packet_loss_pct": 0.0,
        "jitter_ms": 0,
        "cheat_flag_sim": True,
    }

    with open(config_path, "w", encoding="utf-8") as f:
        json.dump(profiles, f, indent=4)

    print(f"[NET SIMULATOR] Generated Network Profile config at: {config_path}")
    print(
        "[NET SIMULATOR] The C++ Tests will read these values to mock Rollbacks depending on the test instance."
    )


if __name__ == "__main__":
    generate_mp_env_config()
