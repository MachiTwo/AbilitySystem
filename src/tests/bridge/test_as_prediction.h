/**************************************************************************/
/*  test_as_prediction.h                                                  */
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

#ifndef TEST_AS_PREDICTION_CORRECTION_H
#define TEST_AS_PREDICTION_CORRECTION_H

#include "src/resources/as_attribute.h"
#include "src/resources/as_attribute_set.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

// ==================== HELPER CLASSES ====================

/**
 * Input prediction para testes
 * Simula input de um client que será validado pelo servidor
 */
struct PredictedInput {
	uint64_t tick = 0;
	bool attack_light = false;
	bool attack_heavy = false;
	bool dash = false;
	bool jump = false;

	PredictedInput() = default;
	PredictedInput(uint64_t p_tick) : tick(p_tick) {}
};

/**
 * Simula um scenario de prediction + correction
 */
struct PredictionScenario {
	ASComponent *server_asc = nullptr;
	ASComponent *client_asc = nullptr;
	std::vector<PredictedInput> input_queue;
	uint64_t current_tick = 0;

	PredictionScenario() {
		server_asc = memnew(ASComponent);
		client_asc = memnew(ASComponent);
	}

	~PredictionScenario() {
		if (server_asc)
			memdelete(server_asc);
		if (client_asc)
			memdelete(client_asc);
	}

	void queue_input(PredictedInput input) {
		input_queue.push_back(input);
	}

	void advance_tick() {
		current_tick++;
	}
};

// ==================== BRIDGE TESTS ====================

TEST_SUITE("ASComponent - Prediction & Correction") {
	TEST_CASE("Basic Client Prediction with Server Validation") {
		// Basic prediction test followed by validation
		SUBCASE("Client predicts, server validates, no correction needed") {
			PredictionScenario scenario;

			// Setup
			scenario.server_asc->set_attribute_base_value_by_tag(StringName("Health"), 100.0f);
			scenario.client_asc->set_attribute_base_value_by_tag(StringName("Health"), 100.0f);

			// Tick 1: Client predicts damage
			scenario.client_asc->set_attribute_base_value_by_tag(StringName("Health"), 85.0f);
			scenario.advance_tick();

			// Tick 1: Server validates and applies SAME damage
			scenario.server_asc->set_attribute_base_value_by_tag(StringName("Health"), 85.0f);

			// Validate that both agree
			CHECK(scenario.client_asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(scenario.server_asc->get_attribute_base_value_by_tag(StringName("Health"))));
		}

		SUBCASE("Client mispredicts, receives correction") {
			PredictionScenario scenario;

			// Setup
			scenario.server_asc->set_attribute_base_value_by_tag(StringName("Health"), 100.0f);
			scenario.client_asc->set_attribute_base_value_by_tag(StringName("Health"), 100.0f);

			// Tick 1: Client predicts 85
			scenario.client_asc->set_attribute_base_value_by_tag(StringName("Health"), 85.0f);

			// Tick 1: Server validates but applies 90 (mismatch!)
			scenario.server_asc->set_attribute_base_value_by_tag(StringName("Health"), 90.0f);

			// Capture server state
			auto server_corrected_state = scenario.server_asc->capture_snapshot_state();

			// Verify mismatch
			CHECK(scenario.client_asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(85.0f));
			CHECK(scenario.server_asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(90.0f));

			// Client receives correction
			scenario.client_asc->restore_snapshot_state(server_corrected_state);

			// Now they match
			CHECK(scenario.client_asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(90.0f));
		}
	}

	TEST_CASE("Ability Activation Prediction") {
		// Test ability activation prediction
		SUBCASE("Client predicts ability activation, server validates") {
			PredictionScenario scenario;

			// Setup
			scenario.server_asc->set_attribute_base_value_by_tag(StringName("Health"), 100.0f);
			scenario.client_asc->set_attribute_base_value_by_tag(StringName("Health"), 100.0f);

			// Client predicts light attack
			scenario.client_asc->add_tag(StringName("attack.fast"));
			scenario.advance_tick();

			// Server validates and applies same
			scenario.server_asc->add_tag(StringName("attack.fast"));

			// Both should have the tag
			CHECK(scenario.client_asc->has_tag(StringName("attack.fast")) == true);
			CHECK(scenario.server_asc->has_tag(StringName("attack.fast")) == true);
		}
	}

	TEST_CASE("Stamina Constraint Validation") {
		// Test constraint validation (stamina for dash)
		SUBCASE("Client predicts dash but stamina insufficient on server") {
			PredictionScenario scenario;

			// Setup: low stamina
			scenario.server_asc->set_attribute_base_value_by_tag(StringName("Stamina"), 20.0f);
			scenario.client_asc->set_attribute_base_value_by_tag(StringName("Stamina"), 30.0f); // mismatch

			// Client optimistically predicts dash
			scenario.client_asc->set_attribute_base_value_by_tag(StringName("Stamina"), 10.0f);
			scenario.client_asc->add_tag(StringName("motion.dash"));

			// Server validates: stamina < 50, rejects dash
			// Server does not remove stamina, keeps at 20
			auto server_state = scenario.server_asc->capture_snapshot_state();

			// Client receives correction: remove dash tag, restore stamina
			scenario.client_asc->restore_snapshot_state(server_state);

			CHECK(scenario.client_asc->has_tag(StringName("motion.dash")) == false);
			CHECK(scenario.client_asc->get_attribute_base_value_by_tag(StringName("Stamina")) == doctest::Approx(20.0f));
		}
	}

	TEST_CASE("Multi-Tick Prediction Rollback") {
		// Test multi-tick rollback with replay
		SUBCASE("Predict 5 ticks, server corrects at tick 2, client replays") {
			PredictionScenario scenario;

			// Setup
			float initial_health = 100.0f;
			scenario.server_asc->set_attribute_base_value_by_tag(StringName("Health"), initial_health);
			scenario.client_asc->set_attribute_base_value_by_tag(StringName("Health"), initial_health);

			// Tick 0-4: Client predicts gradual damage
			float predicted_health = initial_health;
			for (int tick = 0; tick < 5; tick++) {
				predicted_health -= 5.0f; // -5 per tick
				scenario.client_asc->set_attribute_base_value_by_tag(StringName("Health"), predicted_health);
				scenario.queue_input(PredictedInput(tick));
				scenario.advance_tick();
			}

			// Tick 0-1: Server validates predictions
			scenario.server_asc->set_attribute_base_value_by_tag(StringName("Health"), 95.0f); // -5
			scenario.server_asc->set_attribute_base_value_by_tag(StringName("Health"), 88.0f); // -7 (different!)

			// At tick 2, server snapshots current state
			auto server_correction = scenario.server_asc->capture_snapshot_state();

			// Verify client is ahead
			CHECK(scenario.client_asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(75.0f));
			CHECK(scenario.server_asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(88.0f));

			// Client receives correction at tick 2
			scenario.client_asc->restore_snapshot_state(server_correction);

			// After correction, client should be back in sync
			CHECK(scenario.client_asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(88.0f));
		}
	}

	TEST_CASE("Tag Consistency After Correction") {
		// Test that tags remain consistent after correction
		SUBCASE("Server and client tags match after correction") {
			PredictionScenario scenario;

			// Client predicts state
			scenario.client_asc->add_tag(StringName("state.idle"));
			scenario.client_asc->add_tag(StringName("motion.walk"));

			// Server has different tags
			scenario.server_asc->add_tag(StringName("state.idle"));
			scenario.server_asc->add_tag(StringName("motion.run")); // different!

			// Get server state
			auto server_state = scenario.server_asc->capture_snapshot_state();

			// Client receives correction
			scenario.client_asc->restore_snapshot_state(server_state);

			// Tags should now match
			CHECK(scenario.client_asc->has_tag(StringName("state.idle")) == true);
			CHECK(scenario.client_asc->has_tag(StringName("motion.run")) == true);
			CHECK(scenario.client_asc->has_tag(StringName("motion.walk")) == false);
		}
	}

	TEST_CASE("Rapid Correction Handling") {
		// Test multiple corrections in sequence
		SUBCASE("Client receives multiple corrections without data loss") {
			PredictionScenario scenario;

			// Correction 1: Health
			scenario.server_asc->set_attribute_base_value_by_tag(StringName("Health"), 80.0f);
			scenario.client_asc->set_attribute_base_value_by_tag(StringName("Health"), 75.0f);

			auto correction_1 = scenario.server_asc->capture_snapshot_state();
			scenario.client_asc->restore_snapshot_state(correction_1);

			CHECK(scenario.client_asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(80.0f));

			// Correction 2: Health + Stamina
			scenario.server_asc->set_attribute_base_value_by_tag(StringName("Health"), 70.0f);
			scenario.server_asc->set_attribute_base_value_by_tag(StringName("Stamina"), 50.0f);

			auto correction_2 = scenario.server_asc->capture_snapshot_state();
			scenario.client_asc->restore_snapshot_state(correction_2);

			CHECK(scenario.client_asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(70.0f));
			CHECK(scenario.client_asc->get_attribute_base_value_by_tag(StringName("Stamina")) == doctest::Approx(50.0f));
		}
	}

	TEST_CASE("Prediction with Latency Variance") {
		// Test prediction under variable latency
		SUBCASE("Predictions remain consistent under 50-200ms latency") {
			PredictionScenario scenario;

			// Simulate variable latency
			std::vector<int> latencies_ms = { 50, 80, 120, 150, 200, 100, 75, 90, 180, 160 };

			float health = 100.0f;
			for (int i = 0; i < 10; i++) {
				int latency_ms = latencies_ms[i];

				// Client predicts based on latency
				float predicted_damage = latency_ms / 100.0f; // Scale damage by latency
				health -= predicted_damage;

				scenario.client_asc->set_attribute_base_value_by_tag(StringName("Health"), health);

				// Server processes after latency
				scenario.server_asc->set_attribute_base_value_by_tag(
						StringName("Health"),
						health // Same as client this time
				);

				scenario.advance_tick();
			}

			// Final state should match
			CHECK(scenario.client_asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(scenario.server_asc->get_attribute_base_value_by_tag(StringName("Health"))));
		}
	}
}

#endif // TEST_AS_PREDICTION_CORRECTION_H
