/**************************************************************************/
/*  test_as_effect.h                                                      */
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

#pragma once

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/resources/as_effect.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/resources/as_effect.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("ASEffect API") {
	Ref<ASEffect> effect = memnew(ASEffect);

	SUBCASE("Basic Identity - 3 Variations") {
		// Var 1: Name round-trip
		effect->set_effect_name("Poison");
		CHECK(effect->get_effect_name() == String("Poison"));

		// Var 2: Tag round-trip
		effect->set_effect_tag("Effect.Poison");
		CHECK(effect->get_effect_tag() == StringName("Effect.Poison"));

		// Var 3: Period (periodic tick interval)
		effect->set_period(1.5f);
		CHECK(effect->get_period() == 1.5f);
	}

	SUBCASE("Duration Policy - 3 Variations") {
		// Var 1: Instant — triggers once, no duration
		effect->set_duration_policy(ASEffect::POLICY_INSTANT);
		CHECK(effect->get_duration_policy() == ASEffect::POLICY_INSTANT);

		// Var 2: Duration — fixed time window, requires magnitude
		effect->set_duration_policy(ASEffect::POLICY_DURATION);
		effect->set_duration_magnitude(5.0f);
		CHECK(effect->get_duration_policy() == ASEffect::POLICY_DURATION);
		CHECK(effect->get_duration_magnitude() == 5.0f);

		// Var 3: Infinite — stays active until explicitly removed
		effect->set_duration_policy(ASEffect::POLICY_INFINITE);
		CHECK(effect->get_duration_policy() == ASEffect::POLICY_INFINITE);
	}

	SUBCASE("Modifiers - 3 Variations by Operation") {
		// Var 1: OP_ADD — flat modifier, non-custom
		effect->add_modifier("Health", ASEffect::OP_ADD, -10.0f);
		CHECK(effect->get_modifier_count() == 1);
		CHECK(effect->get_modifier_attribute(0) == StringName("Health"));
		CHECK(effect->get_modifier_operation(0) == ASEffect::OP_ADD);
		CHECK(effect->get_modifier_magnitude(0) == -10.0f);
		CHECK(effect->is_modifier_custom(0) == false);

		// Var 2: OP_MULTIPLY — scaling modifier
		effect->add_modifier("Attack", ASEffect::OP_MULTIPLY, 1.5f);
		CHECK(effect->get_modifier_count() == 2);
		CHECK(effect->get_modifier_operation(1) == ASEffect::OP_MULTIPLY);
		CHECK(effect->get_modifier_magnitude(1) == 1.5f);

		// Var 3: OP_OVERRIDE — custom magnitude flag
		effect->add_modifier("Mana", ASEffect::OP_OVERRIDE, 50.0f, true);
		CHECK(effect->get_modifier_count() == 3);
		CHECK(effect->get_modifier_operation(2) == ASEffect::OP_OVERRIDE);
		CHECK(effect->is_modifier_custom(2) == true);
	}

	SUBCASE("Granted Tags - 3 Variations") {
		// Var 1: Round-trip — verify content identity, not just size
		TypedArray<StringName> tags;
		tags.push_back("State.Poisoned");
		tags.push_back("State.Burning");
		effect->set_granted_tags(tags);
		CHECK(effect->get_granted_tags().size() == 2);
		CHECK(StringName(effect->get_granted_tags()[0]) == StringName("State.Poisoned"));
		CHECK(StringName(effect->get_granted_tags()[1]) == StringName("State.Burning"));

		// Var 2: Replace — old list must be fully replaced, not appended
		TypedArray<StringName> replacement;
		replacement.push_back("State.Wet");
		effect->set_granted_tags(replacement);
		CHECK(effect->get_granted_tags().size() == 1);
		CHECK(StringName(effect->get_granted_tags()[0]) == StringName("State.Wet"));

		// Var 3: Empty — means no tags are granted (valid use case)
		effect->set_granted_tags(TypedArray<StringName>());
		CHECK(effect->get_granted_tags().size() == 0);
	}

	SUBCASE("Activation Required All Tags (AND) - 3 Variations") {
		// Var 1: Round-trip — multiple tags, verify each by name
		TypedArray<StringName> req;
		req.push_back("State.Grounded");
		req.push_back("State.Alive");
		effect->set_activation_required_all_tags(req);
		CHECK(effect->get_activation_required_all_tags().size() == 2);
		CHECK(StringName(effect->get_activation_required_all_tags()[0]) == StringName("State.Grounded"));
		CHECK(StringName(effect->get_activation_required_all_tags()[1]) == StringName("State.Alive"));

		// Var 2: Replace — new list replaces old entirely
		TypedArray<StringName> req2;
		req2.push_back("State.InCombat");
		effect->set_activation_required_all_tags(req2);
		CHECK(effect->get_activation_required_all_tags().size() == 1);
		CHECK(StringName(effect->get_activation_required_all_tags()[0]) == StringName("State.InCombat"));

		// Var 3: Empty — means no ALL requirement (always passes AND check)
		effect->set_activation_required_all_tags(TypedArray<StringName>());
		CHECK(effect->get_activation_required_all_tags().size() == 0);
	}

	SUBCASE("Activation Required Any Tags (OR) - 3 Variations") {
		// Var 1: Round-trip — triple option list, verify content
		TypedArray<StringName> req;
		req.push_back("buff.fire");
		req.push_back("buff.water");
		req.push_back("buff.earth");
		effect->set_activation_required_any_tags(req);
		CHECK(effect->get_activation_required_any_tags().size() == 3);
		CHECK(StringName(effect->get_activation_required_any_tags()[0]) == StringName("buff.fire"));

		// Var 2: Replace — narrow down to a single option
		TypedArray<StringName> req2;
		req2.push_back("buff.fire");
		effect->set_activation_required_any_tags(req2);
		CHECK(effect->get_activation_required_any_tags().size() == 1);
		CHECK(StringName(effect->get_activation_required_any_tags()[0]) == StringName("buff.fire"));

		// Var 3: Empty — means no OR requirement (always passes OR check)
		effect->set_activation_required_any_tags(TypedArray<StringName>());
		CHECK(effect->get_activation_required_any_tags().size() == 0);
	}

	SUBCASE("Activation Blocked Any Tags (OR) - 3 Variations") {
		// Var 1: Round-trip — two blockers, verify by name
		TypedArray<StringName> block;
		block.push_back("State.Immune");
		block.push_back("State.Dead");
		effect->set_activation_blocked_any_tags(block);
		CHECK(effect->get_activation_blocked_any_tags().size() == 2);
		CHECK(StringName(effect->get_activation_blocked_any_tags()[0]) == StringName("State.Immune"));
		CHECK(StringName(effect->get_activation_blocked_any_tags()[1]) == StringName("State.Dead"));

		// Var 2: Replace — swap to a different single blocker
		TypedArray<StringName> block2;
		block2.push_back("State.Ethereal");
		effect->set_activation_blocked_any_tags(block2);
		CHECK(effect->get_activation_blocked_any_tags().size() == 1);
		CHECK(StringName(effect->get_activation_blocked_any_tags()[0]) == StringName("State.Ethereal"));

		// Var 3: Empty — means no OR block (nothing blocks via this rule)
		effect->set_activation_blocked_any_tags(TypedArray<StringName>());
		CHECK(effect->get_activation_blocked_any_tags().size() == 0);
	}

	SUBCASE("Activation Blocked All Tags (AND) - 3 Variations") {
		// Var 1: Round-trip — combo condition, verify content
		TypedArray<StringName> block;
		block.push_back("action.jumping");
		block.push_back("action.attacking");
		effect->set_activation_blocked_all_tags(block);
		CHECK(effect->get_activation_blocked_all_tags().size() == 2);
		CHECK(StringName(effect->get_activation_blocked_all_tags()[0]) == StringName("action.jumping"));
		CHECK(StringName(effect->get_activation_blocked_all_tags()[1]) == StringName("action.attacking"));

		// Var 2: Replace — change to a different AND combo
		TypedArray<StringName> block2;
		block2.push_back("state.stunned");
		block2.push_back("state.rooted");
		block2.push_back("state.silenced");
		effect->set_activation_blocked_all_tags(block2);
		CHECK(effect->get_activation_blocked_all_tags().size() == 3);
		CHECK(StringName(effect->get_activation_blocked_all_tags()[0]) == StringName("state.stunned"));

		// Var 3: Empty — means no AND block (nothing blocks via this rule)
		effect->set_activation_blocked_all_tags(TypedArray<StringName>());
		CHECK(effect->get_activation_blocked_all_tags().size() == 0);
	}

	SUBCASE("Stacking Policy - 3 Variations") {
		// Var 1: New Instance — each application is independent
		effect->set_stacking_policy(ASEffect::STACK_NEW_INSTANCE);
		CHECK(effect->get_stacking_policy() == ASEffect::STACK_NEW_INSTANCE);

		// Var 2: Override — new application resets the timer
		effect->set_stacking_policy(ASEffect::STACK_OVERRIDE);
		CHECK(effect->get_stacking_policy() == ASEffect::STACK_OVERRIDE);

		// Var 3: Intensity + Duration — cover remaining two policies
		effect->set_stacking_policy(ASEffect::STACK_INTENSITY);
		CHECK(effect->get_stacking_policy() == ASEffect::STACK_INTENSITY);
		effect->set_stacking_policy(ASEffect::STACK_DURATION);
		CHECK(effect->get_stacking_policy() == ASEffect::STACK_DURATION);
	}

	SUBCASE("Requirements Management - 3 Variations") {
		// Var 1: Round-trip — single requirement, verify content
		effect->add_requirement("Intelligence", 20.0f);
		CHECK(effect->get_requirement_count() == 1);
		CHECK(effect->get_requirement_attribute(0) == StringName("Intelligence"));
		CHECK(effect->get_requirement_amount(0) == 20.0f);

		// Var 2: Multiple — accumulates independently, verify each
		effect->add_requirement("Strength", 10.0f);
		effect->add_requirement("Dexterity", 15.0f);
		CHECK(effect->get_requirement_count() == 3);
		CHECK(effect->get_requirement_attribute(1) == StringName("Strength"));
		CHECK(effect->get_requirement_amount(2) == 15.0f);

		// Var 3: Clear — reset to zero requirements
		effect->set_requirements_count(0);
		CHECK(effect->get_requirement_count() == 0);
	}
}
