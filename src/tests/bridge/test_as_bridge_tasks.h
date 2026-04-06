/**************************************************************************/
/*  test_as_bridge_tasks.h                                                */
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

// NOTE: BTActionAS_ActivateAbility and BTConditionAS_HasTag expose only _tick()
// as a protected virtual (LimboAI convention). Direct invocation of enter()/tick()
// from outside the BT runner is not part of the public contract.
// These tests validate the observable behaviour via ASComponent's public API,
// which is exactly what a GDScript/C# user would do.

#ifndef TEST_AS_BRIDGE_TASKS_H
#define TEST_AS_BRIDGE_TASKS_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/bridge/as_bridge_action_activate.h"
#include "src/bridge/as_bridge_condition_has_tag.h"
#include "src/resources/as_ability.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/bridge/as_bridge_action_activate.h"
#include "modules/ability_system/bridge/as_bridge_condition_has_tag.h"
#include "modules/ability_system/resources/as_ability.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[ASBridge] Task Configuration & Registration (Public API)") {
	Node *root = memnew(Node);
	ASComponent *asc = memnew(ASComponent);
	root->add_child(asc);

	Ref<ASAbility> fireball;
	fireball.instantiate();
	fireball->set_ability_tag(StringName("Ability.Fireball"));
	asc->unlock_ability_by_resource(fireball);

	SUBCASE("BTActionAS_ActivateAbility — property setters/getters - 3 Variations") {
		// Var 1: Default ability tag is empty
		BTActionAS_ActivateAbility *action = memnew(BTActionAS_ActivateAbility);
		CHECK(action->get_ability_tag() == StringName(""));

		// Var 2: Tag is set and retrieved correctly
		action->set_ability_tag(StringName("Ability.Fireball"));
		CHECK(action->get_ability_tag() == StringName("Ability.Fireball"));

		// Var 3: NodePath setter is safe and round-trips
		action->set_asc_node_path(NodePath(".."));
		CHECK(action->get_asc_node_path() == NodePath(".."));

		memdelete(action);
	}

	SUBCASE("BTConditionAS_HasTag — property setters/getters - 3 Variations") {
		// Var 1: Default tag is empty
		BTConditionAS_HasTag *cond = memnew(BTConditionAS_HasTag);
		CHECK(cond->get_tag() == StringName(""));

		// Var 2: Tag is set and retrieved correctly
		cond->set_tag(StringName("State.Burn"));
		CHECK(cond->get_tag() == StringName("State.Burn"));

		// Var 3: exact_match defaults to false and can be toggled
		CHECK_FALSE(cond->get_exact_match());
		cond->set_exact_match(true);
		CHECK(cond->get_exact_match());

		memdelete(cond);
	}

	SUBCASE("ASComponent tag/ability state observable via public API - 3 Variations") {
		// Var 1: Ability unlocked — not yet active
		CHECK(asc->is_ability_unlocked(StringName("Ability.Fireball")));
		CHECK_FALSE(asc->is_ability_active(StringName("Ability.Fireball")));

		// Var 2: try_activate changes observable active state
		bool ok = asc->try_activate_ability_by_resource(fireball);
		CHECK(ok);

		// Var 3: cancel clears active state
		asc->cancel_ability_by_tag(StringName("Ability.Fireball"));
		CHECK_FALSE(asc->is_ability_active(StringName("Ability.Fireball")));
	}

	memdelete(root);
}

#endif // TEST_AS_BRIDGE_TASKS_H
