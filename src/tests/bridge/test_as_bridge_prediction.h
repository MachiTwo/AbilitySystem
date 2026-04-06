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

// NOTE: ASComponent::current_tick is internal (protected). The observable effect
// of a rollback is the restoration of public state (tags, attributes).
// These tests validate that contract via ASComponent's public API only.

#ifndef TEST_AS_BRIDGE_PREDICTION_H
#define TEST_AS_BRIDGE_PREDICTION_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/bridge/as_bridge_prediction.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/bridge/as_bridge_prediction.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[ASBridgePrediction] Tag State Rollback (Public API)") {
	Node *root = memnew(Node);
	ASComponent *asc = memnew(ASComponent);
	ASBridgePrediction *predictor = memnew(ASBridgePrediction);

	root->add_child(asc);
	root->add_child(predictor);

	predictor->setup(asc, nullptr); // No AI node needed for pure ASC rollback tests

	SUBCASE("capture_tick / rollback_to_tick — tag state observable - 3 Variations") {
		// Var 1: Tag added after capture is rolled back
		asc->add_tag(StringName("State.Active"));
		predictor->capture_tick(100);

		asc->remove_tag(StringName("State.Active"));
		CHECK_FALSE(asc->has_tag(StringName("State.Active")));

		predictor->rollback_to_tick(100);
		CHECK(asc->has_tag(StringName("State.Active")));

		// Var 2: Tag absent at capture is absent after rollback
		asc->remove_tag(StringName("State.Active"));
		predictor->capture_tick(110);
		asc->add_tag(StringName("State.Active")); // Add AFTER capture
		predictor->rollback_to_tick(110);
		CHECK_FALSE(asc->has_tag(StringName("State.Active")));

		// Var 3: Rollback to non-existent tick does not crash (graceful no-op)
		predictor->rollback_to_tick(9999);
		// No assertion on specific state — just must not crash
	}

	SUBCASE("set_max_history / get_max_history — configuration - 3 Variations") {
		// Var 1: Default is 128
		CHECK(predictor->get_max_history() == 128);

		// Var 2: Can be reduced
		predictor->set_max_history(5);
		CHECK(predictor->get_max_history() == 5);

		// Var 3: Can be increased beyond default
		predictor->set_max_history(256);
		CHECK(predictor->get_max_history() == 256);
	}

	SUBCASE("capture_tick overflow / history limit compliance - 3 Variations") {
		predictor->set_max_history(3);

		// Var 1: Capture up to limit — no crash
		predictor->capture_tick(200);
		predictor->capture_tick(201);
		predictor->capture_tick(202);

		// Var 2: Exceeding limit — oldest entry is evicted, rollback to it fails gracefully
		predictor->capture_tick(203);
		predictor->rollback_to_tick(200); // Evicted tick — graceful no-op

		// Var 3: Recent tick within limit is still restorable
		asc->add_tag(StringName("State.Burning"));
		predictor->capture_tick(204);
		asc->remove_tag(StringName("State.Burning"));
		predictor->rollback_to_tick(204);
		CHECK(asc->has_tag(StringName("State.Burning")));
	}

	memdelete(root);
}

#endif // TEST_AS_BRIDGE_PREDICTION_H
