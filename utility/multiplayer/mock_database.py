import json
import time

# A mock database system to test if the ASComponent can handle async save/loading.
# In a real game (like Zyris engine), the engine's DB takes care of this.
# Here, we offer a Python endpoint simulation for doctests targeting the "Persistence" layer.


class MockDB:
    def __init__(self):
        self.storage = {}

    def save_player_state(self, player_id, attributes_json, tags_list):
        print(f"[MOCK DB] Saving State for Player {player_id}...")
        time.sleep(0.5)  # Simulate DB hit latency

        self.storage[player_id] = {
            "attributes": attributes_json,
            "tags": tags_list,
            "saved_at": time.time(),
        }
        print(f"[MOCK DB] State Saved for Player {player_id}.")
        return True

    def load_player_state(self, player_id):
        print(f"[MOCK DB] Fetching State for Player {player_id}...")
        time.sleep(0.3)

        if player_id in self.storage:
            print("[MOCK DB] State Loaded.")
            return self.storage[player_id]
        print("[MOCK DB] No record found.")
        return None


if __name__ == "__main__":
    import sys

    db = MockDB()

    if len(sys.argv) > 1 and sys.argv[1] == "test":
        db.save_player_state("PL_01", {"hp": 100}, ["state.alive"])
        print(json.dumps(db.load_player_state("PL_01"), indent=2))
