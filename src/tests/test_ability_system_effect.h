/**************************************************************************/
/*  test_ability_system_effect.h                                          */
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
#include "src/resources/ability_system_effect.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/tests/doctest.h"
#endif

using namespace godot;

TEST_CASE("AbilitySystemEffect API") {
	Ref<AbilitySystemEffect> effect = memnew(AbilitySystemEffect);

	SUBCASE("Basic Properties") {
		effect->set_effect_name("Poison");
		CHECK(effect->get_effect_name() == String("Poison"));

		effect->set_effect_tag("Effect.Poison");
		CHECK(effect->get_effect_tag() == StringName("Effect.Poison"));

		effect->set_duration_policy(AbilitySystemEffect::POLICY_DURATION);
		CHECK(effect->get_duration_policy() == AbilitySystemEffect::POLICY_DURATION);

		effect->set_duration_magnitude(10.0f);
		CHECK(effect->get_duration_magnitude() == 10.0f);

		effect->set_period(1.0f);
		CHECK(effect->get_period() == 1.0f);
	}

	SUBCASE("Modifiers Management") {
		effect->add_modifier("Health", AbilitySystemEffect::OP_ADD, -10.0f);
		CHECK(effect->get_modifier_count() == 1);
		CHECK(effect->get_modifier_attribute(0) == StringName("Health"));
		CHECK(effect->get_modifier_operation(0) == AbilitySystemEffect::OP_ADD);
		CHECK(effect->get_modifier_magnitude(0) == -10.0f);
		CHECK(effect->is_modifier_custom(0) == false);

		effect->add_modifier("Mana", AbilitySystemEffect::OP_OVERRIDE, 50.0f, true);
		CHECK(effect->get_modifier_count() == 2);
		CHECK(effect->is_modifier_custom(1) == true);
	}

	SUBCASE("Tags Management") {
		TypedArray<StringName> granted;
		granted.push_back("State.Poisoned");
		effect->set_granted_tags(granted);
		CHECK(effect->get_granted_tags().size() == 1);

		TypedArray<StringName> blocked;
		blocked.push_back("State.Immune");
		effect->set_activation_blocked_tags(blocked);
		CHECK(effect->get_activation_blocked_tags().size() == 1);
	}

	SUBCASE("Stacking Policy") {
		effect->set_stacking_policy(AbilitySystemEffect::STACK_INTENSITY);
		CHECK(effect->get_stacking_policy() == AbilitySystemEffect::STACK_INTENSITY);
	}

	SUBCASE("Requirements Management") {
		effect->add_requirement("Intelligence", 20.0f);
		CHECK(effect->get_requirement_count() == 1);
		CHECK(effect->get_requirement_attribute(0) == StringName("Intelligence"));
		CHECK(effect->get_requirement_amount(0) == 20.0f);

		effect->set_requirements_count(0);
		CHECK(effect->get_requirement_count() == 0);
	}
}
