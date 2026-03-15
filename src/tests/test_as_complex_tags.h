/**************************************************************************/
/*  test_as_complex_tags.h                                                */
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

#ifndef TEST_AS_COMPLEX_TAGS_H
#define TEST_AS_COMPLEX_TAGS_H

#include "doctest.h"
#include "src/core/as_ability_spec.h"
#include "src/core/as_tag_spec.h"
#include "src/resources/as_ability.h"
#include "src/scene/as_component.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AbilitySystem] Formalized Tag Logic (300% Coverage)") {
	ASComponent *component = memnew(ASComponent);
	Ref<ASTagSpec> tags = component->get_owned_tags();

	Ref<ASAbility> ability;
	ability.instantiate();
	ability->set_ability_tag("test.ability");

	Ref<ASAbilitySpec> spec;
	spec.instantiate();
	spec->init(ability);
	spec->set_owner(component);

	SUBCASE("Required ALL (AND Logic) - 3 Variations") {
		TypedArray<StringName> req_all;
		req_all.push_back("state.a");
		req_all.push_back("state.b");
		req_all.push_back("state.c");
		ability->set_activation_required_all_tags(req_all);

		// Var 1: Incomplete Set (Fail)
		component->add_tag("state.a");
		component->add_tag("state.b");
		CHECK(ability->can_activate_ability(component, spec) == false);

		// Var 2: Exact Set (Success)
		component->add_tag("state.c");
		CHECK(ability->can_activate_ability(component, spec) == true);

		// Var 3: Hierarchical (Success) - state.a.child counts as state.a
		component->remove_tag("state.a");
		component->add_tag("state.a.child");
		CHECK(ability->can_activate_ability(component, spec) == true);

		component->clear_effects(); // Reset tags from effects if any
		tags->remove_all_tags();
	}

	SUBCASE("Required ANY (OR Logic) - 3 Variations") {
		TypedArray<StringName> req_any;
		req_any.push_back("buff.fire");
		req_any.push_back("buff.water");
		req_any.push_back("buff.earth");
		ability->set_activation_required_any_tags(req_any);

		// Var 1: Single Match (Success)
		component->add_tag("buff.fire");
		CHECK(ability->can_activate_ability(component, spec) == true);

		// Var 2: Different Match (Success)
		component->remove_tag("buff.fire");
		component->add_tag("buff.water");
		CHECK(ability->can_activate_ability(component, spec) == true);

		// Var 3: Match with Extra Noise (Success)
		component->add_tag("debuff.poison");
		CHECK(ability->can_activate_ability(component, spec) == true);

		tags->remove_all_tags();
	}

	SUBCASE("Blocked ANY (OR Logic) - 3 Variations") {
		TypedArray<StringName> block_any;
		block_any.push_back("state.stunned");
		block_any.push_back("state.silenced");
		block_any.push_back("state.dead");
		ability->set_activation_blocked_any_tags(block_any);

		// Var 1: Solo Block (Fail)
		component->add_tag("state.stunned");
		CHECK(ability->can_activate_ability(component, spec) == false);

		// Var 2: Hierarchical Block (Fail) - state.dead.ghost counts as state.dead
		component->remove_tag("state.stunned");
		component->add_tag("state.dead.ghost");
		CHECK(ability->can_activate_ability(component, spec) == false);

		// Var 3: Multiple Blocks (Fail)
		component->add_tag("state.silenced");
		CHECK(ability->can_activate_ability(component, spec) == false);

		tags->remove_all_tags();
	}

	SUBCASE("Blocked ALL (AND Logic) - 3 Variations") {
		TypedArray<StringName> block_all;
		block_all.push_back("action.jumping");
		block_all.push_back("action.attacking");
		block_all.push_back("action.dashing");
		ability->set_activation_blocked_all_tags(block_all);

		// Var 1: Two out of Three (Success - NOT blocked)
		component->add_tag("action.jumping");
		component->add_tag("action.attacking");
		CHECK(ability->can_activate_ability(component, spec) == true);

		// Var 2: Three out of Three (Fail - BLOCKED)
		component->add_tag("action.dashing");
		CHECK(ability->can_activate_ability(component, spec) == false);

		// Var 3: Extreme Multi-match (Fail - BLOCKED)
		component->add_tag("extra.state");
		CHECK(ability->can_activate_ability(component, spec) == false);

		tags->remove_all_tags();
	}

	memdelete(component);
}

#endif // TEST_AS_COMPLEX_TAGS_H
