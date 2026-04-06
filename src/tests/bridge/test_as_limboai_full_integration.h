/**************************************************************************/
/*  test_as_limboai_full_integration.h                                    */
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

// NOTE on LimboAI API:
//  - ASComponent::resolve() is protected (internal). Use asc->has_tag() etc. directly.
//  - BehaviorTree::instantiate() requires 4 args: (Node*, Ref<Blackboard>, Node*, Node*).
//  - BTInstance has no public tick() — use BTPlayer or the BT runner pipeline.
//  - LimboHSM::transition_to() does not exist; use change_active_state().
//  - ASBridgePrediction::capture_tick()/rollback_to_tick() are the public rollback API.

#ifndef TEST_AS_LIMBOAI_FULL_INTEGRATION_H
#define TEST_AS_LIMBOAI_FULL_INTEGRATION_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/bridge/as_bridge_prediction.h"
#include "src/bridge/as_bridge_state.h"
#include "src/limboai/bt/tasks/composites/bt_sequence.h"
#include "src/limboai/hsm/limbo_hsm.h"
#include "src/limboai/hsm/limbo_state.h"
#include "src/resources/as_ability.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/bridge/as_bridge_prediction.h"
#include "modules/ability_system/bridge/as_bridge_state.h"
#include "modules/ability_system/limboai/bt/tasks/composites/bt_sequence.h"
#include "modules/ability_system/limboai/hsm/limbo_hsm.h"
#include "modules/ability_system/limboai/hsm/limbo_state.h"
#include "modules/ability_system/resources/as_ability.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[ASBridge] LimboHSM Integration (Public API)") {
	Node *agent = memnew(Node);
	ASComponent *asc = memnew(ASComponent);
	agent->add_child(asc);

	Ref<ASAbility> fireball;
	fireball.instantiate();
	fireball->set_ability_tag(StringName("Ability.Fireball"));
	asc->unlock_ability_by_resource(fireball);

	SUBCASE("LimboHSM state transitions via change_active_state - 3 Variations") {
		LimboHSM *hsm = memnew(LimboHSM);
		agent->add_child(hsm);

		LimboState *idle_state = memnew(LimboState);
		idle_state->set_name("Idle");
		hsm->add_child(idle_state);

		LimboState *hit_state = memnew(LimboState);
		hit_state->set_name("Hit");
		hsm->add_child(hit_state);

		hsm->set_initial_state(idle_state);
		hsm->initialize(agent);
		hsm->set_active(true);

		// Var 1: Initial state is Idle
		CHECK(hsm->get_active_state() == idle_state);

		// Var 2: Transition to Hit
		hsm->change_active_state(hit_state);
		CHECK(hsm->get_active_state() == hit_state);

		// Var 3: Transition back to Idle
		hsm->change_active_state(idle_state);
		CHECK(hsm->get_active_state() == idle_state);

		hsm->set_active(false);
	}

	SUBCASE("LimboHSM capture_state / restore_state - 3 Variations") {
		LimboHSM *hsm = memnew(LimboHSM);
		agent->add_child(hsm);

		LimboState *idle_state = memnew(LimboState);
		idle_state->set_name("Idle");
		hsm->add_child(idle_state);

		LimboState *hit_state = memnew(LimboState);
		hit_state->set_name("Hit");
		hsm->add_child(hit_state);

		hsm->set_initial_state(idle_state);
		hsm->initialize(agent);
		hsm->set_active(true);

		// Var 1: Captured state restores correctly
		Dictionary snapshot = hsm->capture_state();
		hsm->change_active_state(hit_state);
		CHECK(hsm->get_active_state() == hit_state);
		hsm->restore_state(snapshot);
		CHECK(hsm->get_active_state() == idle_state);

		// Var 2: Snapshot of Hit state restores to Hit
		hsm->change_active_state(hit_state);
		Dictionary snap2 = hsm->capture_state();
		hsm->change_active_state(idle_state);
		hsm->restore_state(snap2);
		CHECK(hsm->get_active_state() == hit_state);

		// Var 3: Empty dictionary restore is a safe no-op
		hsm->restore_state(Dictionary());
		// Still alive, no crash — current state unchanged
		CHECK(hsm->get_active_state() != nullptr);

		hsm->set_active(false);
	}

	SUBCASE("ASBridgePrediction — synchronized rollback observable via tags - 3 Variations") {
		LimboHSM *hsm = memnew(LimboHSM);
		agent->add_child(hsm);

		LimboState *idle_state = memnew(LimboState);
		idle_state->set_name("Idle");
		hsm->add_child(idle_state);
		hsm->set_initial_state(idle_state);
		hsm->initialize(agent);
		hsm->set_active(true);

		ASBridgePrediction *predictor = memnew(ASBridgePrediction);
		agent->add_child(predictor);
		predictor->setup(asc, hsm);

		// Var 1: Tag present at capture is restored after rollback
		asc->add_tag(StringName("State.Ready"));
		predictor->capture_tick(300);
		asc->remove_tag(StringName("State.Ready"));
		predictor->rollback_to_tick(300);
		CHECK(asc->has_tag(StringName("State.Ready")));

		// Var 2: Tag absent at capture is absent after rollback
		asc->remove_tag(StringName("State.Ready"));
		predictor->capture_tick(301);
		asc->add_tag(StringName("State.Ready"));
		predictor->rollback_to_tick(301);
		CHECK_FALSE(asc->has_tag(StringName("State.Ready")));

		// Var 3: Rollback to missing tick is a graceful no-op
		predictor->rollback_to_tick(9999);
		// Must not crash

		hsm->set_active(false);
	}

	memdelete(agent);
}

#endif // TEST_AS_LIMBOAI_FULL_INTEGRATION_H
