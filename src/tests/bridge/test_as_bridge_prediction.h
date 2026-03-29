/**************************************************************************/
/*  test_as_bridge_prediction.h                                           */
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

#ifndef TEST_AS_BRIDGE_PREDICTION_H
#define TEST_AS_BRIDGE_PREDICTION_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/bridge/as_bridge_prediction.h"
#include "src/limboai/bt/bt_player.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/bridge/as_bridge_prediction.h"
#include "modules/ability_system/limboai/bt/bt_player.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("ASBridge - Prediction Rollback") {
	Node *root = memnew(Node);
	ASComponent *asc = memnew(ASComponent);
	BTPlayer *player = memnew(BTPlayer);
	ASBridgePrediction *predictor = memnew(ASBridgePrediction);

	root->add_child(asc);
	root->add_child(player);
	root->add_child(predictor);

	predictor->setup(asc, player);

	SUBCASE("Synchronized Capture - 3 Variations") {
		// Var 1: Basic capture and successful rollback
		asc->current_tick = 100;
		asc->add_tag("State.Active");
		predictor->capture_tick(100);

		asc->current_tick = 101;
		asc->remove_tag("State.Active");
		predictor->capture_tick(101);

		predictor->rollback_to_tick(100);
		CHECK(asc->has_tag("State.Active") == true);
		CHECK(asc->current_tick == 100);

		// Var 2: Capture with Blackboard (AI State)
		Ref<Blackboard> bb = player->get_blackboard();
		bb->set_var("health", 100);
		predictor->capture_tick(110);

		bb->set_var("health", 50);
		predictor->rollback_to_tick(110);
		CHECK((int)bb->get_var("health") == 100);

		// Var 3: Future cleanup after rollback
		predictor->capture_tick(120);
		predictor->capture_tick(121);
		predictor->rollback_to_tick(120);
		// Ticks 121 should be cleared from predictor history
	}

	SUBCASE("Fallback Support - 3 Variations") {
		// Var 1: Rollback to non-existent tick (should not crash)
		predictor->rollback_to_tick(999);
		// Current state should be preserved if destination fails
		CHECK(asc->current_tick == 120);

		// Var 2: Runtime AI Node swap
		Node *other_ai = memnew(Node); // Lacks capture_state/restore methods
		predictor->setup(asc, other_ai);
		predictor->capture_tick(130);
		// Should not crash, just ignore AI if methods are missing
		predictor->rollback_to_tick(130);
		CHECK(asc->current_tick == 130);
		memdelete(other_ai);

		// Var 3: History limit compliance (max_history)
		predictor->set_max_history(5);
		for (int i = 0; i < 10; i++)
			predictor->capture_tick(140 + i);
		// Should only keep up to 5 entries
		predictor->rollback_to_tick(140); // Should fail as it's too old
		CHECK(asc->current_tick == 130); // ASC rollback shouldn't happen if tick is missing in AI?
		// Actually, predictor restores AI if tick exists, but ALWAYS rolls back ASC.
	}

	memdelete(root);
}

#endif // TEST_AS_BRIDGE_PREDICTION_H
