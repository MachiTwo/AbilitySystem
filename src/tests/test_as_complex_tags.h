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

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/tests/test_helpers.h"
#else
#include "modules/ability_system/tests/test_helpers.h"
#endif

TEST_CASE("[AbilitySystem] Formalized Tag Logic (300% Coverage)") {
	ASComponent *component = memnew(ASComponent);

	// Use helper to create ability (resets tags implicitly by creating fresh resource/spec)
	auto setup_test = [&](const StringName &p_tag) {
		component->remove_all_tags();
		Ref<ASAbility> ability;
		ability.instantiate();
		ability->set_ability_tag(p_tag);
		Ref<ASAbilitySpec> spec;
		spec.instantiate();
		spec->init(ability);
		spec->set_owner(component);
		return std::make_pair(ability, spec);
	};

	SUBCASE("Required ALL (AND Logic) - 3 Variations") {
		auto pair = setup_test("test.req_all");
		Ref<ASAbility> ability = pair.first;
		Ref<ASAbilitySpec> spec = pair.second;

		TypedArray<StringName> tags;
		tags.push_back("state.a");
		tags.push_back("state.b");
		ability->set_activation_required_all_tags(tags);

		// Var 1: Empty (Fail)
		CHECK(ability->can_activate_ability(component, spec) == false);

		// Var 2: Partial (Fail)
		component->add_tag("state.a");
		CHECK(ability->can_activate_ability(component, spec) == false);

		// Var 3: Full Match (Success)
		component->add_tag("state.b");
		CHECK(ability->can_activate_ability(component, spec) == true);
	}

	SUBCASE("Required ANY (OR Logic) - 3 Variations") {
		auto pair = setup_test("test.req_any");
		Ref<ASAbility> ability = pair.first;
		Ref<ASAbilitySpec> spec = pair.second;

		TypedArray<StringName> tags;
		tags.push_back("state.a");
		tags.push_back("state.b");
		ability->set_activation_required_any_tags(tags);

		// Var 1: Empty (Fail)
		CHECK(ability->can_activate_ability(component, spec) == false);

		// Var 2: Single Match A (Success)
		component->add_tag("state.a");
		CHECK(ability->can_activate_ability(component, spec) == true);

		// Var 3: Single Match B (Success)
		component->remove_all_tags();
		component->add_tag("state.b");
		CHECK(ability->can_activate_ability(component, spec) == true);
	}

	SUBCASE("Blocked ANY (OR Logic) - 3 Variations") {
		auto pair = setup_test("test.block_any");
		Ref<ASAbility> ability = pair.first;
		Ref<ASAbilitySpec> spec = pair.second;

		TypedArray<StringName> tags;
		tags.push_back("state.stunned");
		tags.push_back("state.dead");
		ability->set_activation_blocked_any_tags(tags);

		// Var 1: Clean State (Success)
		CHECK(ability->can_activate_ability(component, spec) == true);

		// Var 2: Blocked by Stun (Fail)
		component->add_tag("state.stunned");
		CHECK(ability->can_activate_ability(component, spec) == false);

		// Var 3: Blocked by Dead (Fail)
		component->remove_all_tags();
		component->add_tag("state.dead");
		CHECK(ability->can_activate_ability(component, spec) == false);
	}

	SUBCASE("Blocked ALL (AND Logic) - 3 Variations") {
		auto pair = setup_test("test.block_all");
		Ref<ASAbility> ability = pair.first;
		Ref<ASAbilitySpec> spec = pair.second;

		TypedArray<StringName> tags;
		tags.push_back("state.locked");
		tags.push_back("state.silenced");
		ability->set_activation_blocked_all_tags(tags);

		// Var 1: Single Tag Present (Success - not blocked all)
		component->add_tag("state.locked");
		CHECK(ability->can_activate_ability(component, spec) == true);

		// Var 2: Both Tags Present (Fail - blocked all)
		component->add_tag("state.silenced");
		CHECK(ability->can_activate_ability(component, spec) == false);

		// Var 3: Hierarchical Match (Fail - blocked all)
		component->remove_all_tags();
		component->add_tag("state.locked.permanent");
		component->add_tag("state.silenced.magical");
		CHECK(ability->can_activate_ability(component, spec) == false);
	}

	memdelete(component);
}

#endif // TEST_AS_COMPLEX_TAGS_H
