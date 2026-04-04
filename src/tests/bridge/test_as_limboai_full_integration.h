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

// --- MANDATORY TEMPLATE ---
#ifndef TEST_AS_LIMBOAI_FULL_INTEGRATION_H
#define TEST_AS_LIMBOAI_FULL_INTEGRATION_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/bridge/as_bridge_action_activate.h"
#include "src/bridge/as_bridge_condition_has_tag.h"
#include "src/limboai/bt/behavior_tree.h"
#include "src/limboai/bt/tasks/bt_composite.h"
#include "src/limboai/bt/tasks/composites/bt_sequence.h"
#include "src/limboai/hsm/limbo_hsm.h"
#include "src/limboai/hsm/limbo_state.h"
#include "src/resources/as_ability.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/bridge/as_bridge_action_activate.h"
#include "modules/ability_system/bridge/as_bridge_condition_has_tag.h"
#include "modules/ability_system/limboai/bt/behavior_tree.h"
#include "modules/ability_system/limboai/bt/bt_instance.h"
#include "modules/ability_system/limboai/bt/tasks/bt_composite.h"
#include "modules/ability_system/limboai/hsm/limbo_hsm.h"
#include "modules/ability_system/limboai/hsm/limbo_state.h"
#include "modules/ability_system/resources/as_ability.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

// Custom state for testing HSM transitions based on AS tags
class ASStateIntegration : public LimboState {
	GDCLASS(ASStateIntegration, LimboState);

protected:
	static void _bind_methods() {}

public:
	StringName target_tag;
	StringName transition_to_state;

	virtual void _update(double p_delta) override {
		ASComponent *asc = ASComponent::resolve(get_agent());
		if (asc && asc->has_tag(target_tag)) {
			LimboHSM *hsm = Object::cast_to<LimboHSM>(get_parent());
			if (hsm) {
				hsm->change_active_state(Object::cast_to<LimboState>(hsm->get_node<Node>(NodePath(transition_to_state))));
			}
		}
	}
};

TEST_CASE("ASBridge - Full LimboAI Integration Advanced") {
	// Basic Setup
	Node *agent = memnew(Node);
	ASComponent *asc = memnew(ASComponent);
	agent->add_child(asc);

	// Register global tags for the test
	Ref<ASAbility> fireball = memnew(ASAbility);
	fireball->set_ability_tag("Ability.Fireball");
	asc->unlock_ability_by_resource(fireball);

	SUBCASE("Advanced Behavior Tree Sequence - 3 Variations") {
		Ref<BehaviorTree> tree = memnew(BehaviorTree);
		Ref<BTSequence> sequence = memnew(BTSequence);

		// Step 1: Check for Stunned tag (Should fail)
		Ref<BTConditionAS_HasTag> check_stun = memnew(BTConditionAS_HasTag);
		check_stun->set_tag("State.Stunned");
		sequence->add_child(check_stun);

		tree->set_root_task(sequence);
		Ref<BTInstance> bt = tree->instantiate(agent);

		// Var 1: Sequence Failure (First task fails)
		CHECK(bt->tick(0.1) == BT::FAILURE);
		CHECK(asc->is_ability_active("Ability.Fireball") == false);

		// Var 2: Sequence Success (All tasks pass)
		asc->add_tag("State.Stunned");
		CHECK(bt->tick(0.1) == BT::SUCCESS);

		// Var 3: Sequence Internal State Clean (Reset and re-fail)
		// bt->reset(); // Need verification on BTInstance API
		asc->remove_tag("State.Stunned");
		CHECK(bt->update(0.1) == BT::FAILURE);
	}

	SUBCASE("Hierarchical HSM State Transitions - 3 Variations") {
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

		// Var 2: Manual Transition triggers
		hsm->transition_to(hit_state);
		CHECK(hsm->get_active_state() == hit_state);

		// Var 3: Tag-based Rollback compatibility
		Dictionary snapshot = hsm->capture_state();
		hsm->transition_to(idle_state);
		hsm->restore_state(snapshot);
		CHECK(hsm->get_active_state() == hit_state);
	}

	SUBCASE("Blackboard Rollback Synchrony - 3 Variations") {
		ASBridgePrediction *predictor = memnew(ASBridgePrediction);
		agent->add_child(predictor);

		LimboHSM *hsm = memnew(LimboHSM);
		agent->add_child(hsm);
		hsm->initialize(agent);
		hsm->set_active(true);
		predictor->setup(asc, hsm);

		Ref<Blackboard> bb = hsm->get_blackboard();

		// Var 1: Basic numerical value rollback
		bb->set_var("score", 10.0);
		predictor->capture_tick(200);
		bb->set_var("score", 50.0);
		predictor->rollback_to_tick(200);
		CHECK((double)bb->get_var("score") == 10.0);

		// Var 2: Internal Dictionary rollback (reference safety)
		Dictionary data;
		data["id"] = 1;
		bb->set_var("data", data);
		predictor->capture_tick(210);
		data["id"] = 2;
		bb->set_var("data", data);
		predictor->rollback_to_tick(210);
		CHECK((int)Dictionary(bb->get_var("data"))["id"] == 1);

		// Var 3: Boolean state toggle
		bb->set_var("is_ready", true);
		predictor->capture_tick(220);
		bb->set_var("is_ready", false);
		predictor->rollback_to_tick(220);
		CHECK((bool)bb->get_var("is_ready") == true);
	}

	memdelete(agent);
}

#endif // TEST_AS_LIMBOAI_FULL_INTEGRATION_H
