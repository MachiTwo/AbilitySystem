/**************************************************************************/
/*  test_as_bridge_state.h                                                */
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

// NOTE: ASBridgeState inherits LimboState (Node), not RefCounted — use memnew/memdelete.
// set_blackboard() does not exist on ASBridgeState; the blackboard is managed by
// LimboHSM/BTPlayer. These tests validate only the public configuration API.

#ifndef TEST_AS_BRIDGE_STATE_H
#define TEST_AS_BRIDGE_STATE_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/bridge/as_bridge_state.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/bridge/as_bridge_state.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[ASBridgeState] Configuration API (Public)") {
	Node *root = memnew(Node);
	ASComponent *asc = memnew(ASComponent);
	asc->set_name("MainASC");
	root->add_child(asc);

	// ASBridgeState is a Node (LimboState), not a RefCounted — use raw pointer.
	ASBridgeState *bridge = memnew(ASBridgeState);
	root->add_child(bridge);

	SUBCASE("component_alias — setter/getter - 3 Variations") {
		// Var 1: Default alias is 'Self'
		CHECK(bridge->get_component_alias() == StringName("Self"));

		// Var 2: Alias can be changed
		bridge->set_component_alias(StringName("Player"));
		CHECK(bridge->get_component_alias() == StringName("Player"));

		// Var 3: Alias can be reset back to Self
		bridge->set_component_alias(StringName("Self"));
		CHECK(bridge->get_component_alias() == StringName("Self"));
	}

	SUBCASE("required_tags — setter/getter - 3 Variations") {
		// Var 1: Default is empty
		CHECK(bridge->get_required_tags().size() == 0);

		// Var 2: Tags are stored correctly
		TypedArray<StringName> tags;
		tags.push_back(StringName("State.Grounded"));
		tags.push_back(StringName("State.Armed"));
		bridge->set_required_tags(tags);
		CHECK(bridge->get_required_tags().size() == 2);

		// Var 3: Replacing with empty clears them
		bridge->set_required_tags(TypedArray<StringName>());
		CHECK(bridge->get_required_tags().size() == 0);
	}

	SUBCASE("check_tags_on_enter — toggle - 3 Variations") {
		// Var 1: Default is true
		CHECK(bridge->get_check_tags_on_enter() == true);

		// Var 2: Can be disabled
		bridge->set_check_tags_on_enter(false);
		CHECK(bridge->get_check_tags_on_enter() == false);

		// Var 3: Can be re-enabled
		bridge->set_check_tags_on_enter(true);
		CHECK(bridge->get_check_tags_on_enter() == true);
	}

	SUBCASE("transition_event — setter/getter - 3 Variations") {
		// Var 1: Default is empty
		CHECK(bridge->get_transition_event() == StringName(""));

		// Var 2: Event tag is stored
		bridge->set_transition_event(StringName("Event.OnDeath"));
		CHECK(bridge->get_transition_event() == StringName("Event.OnDeath"));

		// Var 3: Overwriting with a different event is fine
		bridge->set_transition_event(StringName("Event.OnHit"));
		CHECK(bridge->get_transition_event() == StringName("Event.OnHit"));
	}

	memdelete(root);
}

#endif // TEST_AS_BRIDGE_STATE_H
