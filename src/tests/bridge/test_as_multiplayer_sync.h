/**************************************************************************/
/*  test_as_multiplayer_sync.h                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef TEST_AS_MULTIPLAYER_SYNC_H
#define TEST_AS_MULTIPLAYER_SYNC_H

#include "src/resources/as_attribute.h"
#include "src/resources/as_attribute_set.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"

#include <cstdlib>
#include <vector>

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

// ==================== HELPER STRUCTURES ====================

/**
 * Simulates a network snapshot for synchronization tests
 * Stores complete ASComponent state at a specific tick
 */
struct NetworkSnapshot {
	uint64_t tick = 0;
	Dictionary component_state;

	NetworkSnapshot() = default;
	NetworkSnapshot(uint64_t p_tick, Dictionary p_state) : tick(p_tick), component_state(p_state) {}
};

/**
 * Simulates a player in a multiplayer environment
 * Stores snapshots and state for synchronization tests
 */
struct SimulatedPlayer {
	int player_id = 0;
	ASComponent *asc = nullptr;
	std::vector<NetworkSnapshot> snapshots;
	uint64_t current_tick = 0;

	SimulatedPlayer(int p_id) : player_id(p_id) {
		asc = memnew(ASComponent);
	}

	~SimulatedPlayer() {
		if (asc)
			memdelete(asc);
	}

	void capture_snapshot() {
		NetworkSnapshot snap(current_tick, asc->capture_snapshot_state());
		snapshots.push_back(snap);
	}

	void advance_tick() {
		current_tick++;
	}
};

// ==================== BRIDGE TESTS ====================

TEST_SUITE("ASComponent - Multiplayer Synchronization") {
	TEST_CASE("Single Player Snapshot Capture & Restore") {
		// Basic capture/restore test for single player
		SUBCASE("Snapshot captures initial state") {
			SimulatedPlayer player(1);

			// Initial setup
			player.asc->set_attribute_base_value_by_tag(StringName("Health"), 100.0f);
			player.asc->set_attribute_base_value_by_tag(StringName("Stamina"), 100.0f);

			// Capture snapshot
			player.capture_snapshot();

			CHECK(player.snapshots.size() == 1);
			CHECK(player.snapshots[0].tick == 0);
			CHECK(player.snapshots[0].component_state.is_empty() == false);
		}

		SUBCASE("Multiple snapshots across ticks") {
			SimulatedPlayer player(2);

			// Snapshot 0
			player.asc->set_attribute_base_value_by_tag(StringName("Health"), 100.0f);
			player.capture_snapshot();
			player.advance_tick();

			// Snapshot 1
			player.asc->set_attribute_base_value_by_tag(StringName("Health"), 80.0f);
			player.capture_snapshot();
			player.advance_tick();

			// Snapshot 2
			player.asc->set_attribute_base_value_by_tag(StringName("Health"), 60.0f);
			player.capture_snapshot();

			CHECK(player.snapshots.size() == 3);
			CHECK(player.current_tick == 2);
		}
	}

	TEST_CASE("Server-Client Snapshot Synchronization") {
		// Synchronization test between server and client
		SUBCASE("Client matches server after snapshot application") {
			SimulatedPlayer server(0); // Authority
			SimulatedPlayer client(1); // Predicted

			// Identical setup
			server.asc->set_attribute_base_value_by_tag(StringName("Health"), 100.0f);
			client.asc->set_attribute_base_value_by_tag(StringName("Health"), 100.0f);

			// Server captures snapshot
			server.capture_snapshot();
			auto server_snap = server.snapshots[0].component_state;

			// Server changes
			server.asc->set_attribute_base_value_by_tag(StringName("Health"), 85.0f);
			server.advance_tick();

			// Client predicts differently
			client.asc->set_attribute_base_value_by_tag(StringName("Health"), 90.0f);
			client.advance_tick();

			// Client receives server correction
			client.asc->restore_snapshot_state(server_snap);

			// Both should have 100 now
			CHECK(client.asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(100.0f));
			CHECK(server.asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(100.0f));
		}
	}

	TEST_CASE("Rollback Determinism") {
		// Test that rollback is deterministic
		SUBCASE("Rollback to earlier tick restores exact state") {
			SimulatedPlayer player(3);

			// Tick 0: Health 100
			player.asc->set_attribute_base_value_by_tag(StringName("Health"), 100.0f);
			player.asc->add_tag(StringName("state.idle"));
			player.capture_snapshot();
			auto snap_0 = player.snapshots[0].component_state;
			player.advance_tick();

			// Tick 1: Health 85
			player.asc->set_attribute_base_value_by_tag(StringName("Health"), 85.0f);
			player.asc->remove_tag(StringName("state.idle"));
			player.asc->add_tag(StringName("state.hurt"));
			player.capture_snapshot();
			player.advance_tick();

			// Tick 2: Health 70
			player.asc->set_attribute_base_value_by_tag(StringName("Health"), 70.0f);
			player.capture_snapshot();

			// Rollback to Tick 0
			player.asc->restore_snapshot_state(snap_0);

			// Verify exact restoration
			CHECK(player.asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(100.0f));
			CHECK(player.asc->has_tag(StringName("state.idle")) == true);
			CHECK(player.asc->has_tag(StringName("state.hurt")) == false);
		}
	}

	TEST_CASE("Multi-Player Cascade Synchronization") {
		// Test with multiple players synchronizing
		SUBCASE("10 players maintain sync with server snapshots") {
			std::vector<SimulatedPlayer> players;
			players.reserve(11);

			// Player 0 = Server (authority)
			players.emplace_back(0);
			// Players 1-10 = Clients
			for (int i = 1; i <= 10; i++) {
				players.emplace_back(i);
			}

			// Simulate 50 ticks
			for (int tick = 0; tick < 50; tick++) {
				// Server faz modificação
				float server_health = 100.0f - (tick * 2.0f);
				players[0].asc->set_attribute_base_value_by_tag(
						StringName("Health"),
						server_health);

				// Clientes predizem diferente
				for (int i = 1; i <= 10; i++) {
					float predicted_health = 100.0f - (tick * 1.8f);
					players[i].asc->set_attribute_base_value_by_tag(
							StringName("Health"),
							predicted_health);
				}

				// Snapshot every 10 ticks
				if (tick % 10 == 0) {
					players[0].capture_snapshot();
					auto server_snap = players[0].snapshots.back().component_state;

					// All clients receive correction
					for (int i = 1; i <= 10; i++) {
						players[i].asc->restore_snapshot_state(server_snap);
					}
				}

				// Advance ticks
				for (auto &player : players) {
					player.advance_tick();
				}
			}

			// Verify all players finished with same state
			float server_final = players[0].asc->get_attribute_base_value_by_tag(StringName("Health"));

			for (int i = 1; i <= 10; i++) {
				float client_health = players[i].asc->get_attribute_base_value_by_tag(StringName("Health"));
				CHECK(client_health == doctest::Approx(server_final));
			}
		}
	}

	TEST_CASE("Tag Synchronization") {
		// Test that tags synchronize correctly
		SUBCASE("Tag state matches after snapshot restore") {
			SimulatedPlayer server(0);
			SimulatedPlayer client(1);

			// Server adds multiple tags
			server.asc->add_tag(StringName("state.idle"));
			server.asc->add_tag(StringName("motion.walk"));
			server.asc->add_tag(StringName("weapon.sword"));
			server.capture_snapshot();

			auto server_snap = server.snapshots[0].component_state;

			// Client has different tags
			client.asc->add_tag(StringName("state.hurt"));
			client.asc->add_tag(StringName("motion.dash"));

			// Apply server snapshot
			client.asc->restore_snapshot_state(server_snap);

			// Verify client has exactly the server tags
			CHECK(client.asc->has_tag(StringName("state.idle")) == true);
			CHECK(client.asc->has_tag(StringName("motion.walk")) == true);
			CHECK(client.asc->has_tag(StringName("weapon.sword")) == true);
			CHECK(client.asc->has_tag(StringName("state.hurt")) == false);
			CHECK(client.asc->has_tag(StringName("motion.dash")) == false);
		}
	}

	TEST_CASE("Latency Variance Impact") {
		// Test behavior with variable latency
		SUBCASE("Snapshots collected at varying latency intervals") {
			SimulatedPlayer player(5);

			// Variable latency (in ticks: 3-12 ticks = 50-200ms @ 60Hz)
			std::vector<int> latency_ticks = { 3, 4, 8, 12, 5, 10, 6, 7, 11, 9 };

			for (int i = 0; i < 10; i++) {
				int latency = latency_ticks[i];

				// Simulate impacto de latência
				float health = 100.0f - (latency * 2.5f);
				player.asc->set_attribute_base_value_by_tag(StringName("Health"), health);
				player.capture_snapshot();

				player.advance_tick();
			}

			// Verify snapshots were captured correctly
			CHECK(player.snapshots.size() == 10);
			CHECK(player.current_tick == 10);

			// Verify health progression
			for (size_t i = 0; i < player.snapshots.size(); i++) {
				CHECK(player.snapshots[i].tick == static_cast<uint64_t>(i));
			}
		}
	}

	TEST_CASE("Disconnection Recovery with Snapshot") {
		// Test disconnection recovery via snapshot
		SUBCASE("Client recovers state after simulated disconnect") {
			SimulatedPlayer server(0);
			SimulatedPlayer client(1);

			// Initial setup
			server.asc->set_attribute_base_value_by_tag(StringName("Health"), 100.0f);
			server.asc->set_attribute_base_value_by_tag(StringName("Stamina"), 100.0f);
			server.asc->add_tag(StringName("state.idle"));
			server.capture_snapshot();

			auto pre_disconnect_snap = server.snapshots[0].component_state;

			// Simulate alterações locais durante "desconexão"
			client.asc->set_attribute_base_value_by_tag(StringName("Health"), 30.0f);
			client.asc->set_attribute_base_value_by_tag(StringName("Stamina"), 20.0f);
			client.asc->remove_tag(StringName("state.idle"));
			client.asc->add_tag(StringName("state.hurt"));

			// "Reconnect" by applying pre-disconnect snapshot
			client.asc->restore_snapshot_state(pre_disconnect_snap);

			// Verify client was restored to known state
			CHECK(client.asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(100.0f));
			CHECK(client.asc->get_attribute_base_value_by_tag(StringName("Stamina")) == doctest::Approx(100.0f));
			CHECK(client.asc->has_tag(StringName("state.idle")) == true);
			CHECK(client.asc->has_tag(StringName("state.hurt")) == false);
		}
	}
}

#endif // TEST_AS_MULTIPLAYER_SYNC_H
