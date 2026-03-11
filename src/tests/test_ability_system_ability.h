/**************************************************************************/
/*  test_ability_system_ability.h                                         */
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
#include "src/resources/ability_system_ability.h"
#include "src/resources/ability_system_effect.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/tests/doctest.h"
#endif

using namespace godot;

TEST_CASE("AbilitySystemAbility API") {
	Ref<AbilitySystemAbility> ability = memnew(AbilitySystemAbility);

	SUBCASE("Basic Properties") {
		ability->set_ability_name("Dash");
		CHECK(ability->get_ability_name() == String("Dash"));

		ability->set_ability_tag("Ability.Dash");
		CHECK(ability->get_ability_tag() == StringName("Ability.Dash"));

		ability->set_cooldown_duration(2.0f);
		CHECK(ability->get_cooldown_duration() == 2.0f);
	}

	SUBCASE("Tags Management") {
		TypedArray<StringName> blocked;
		blocked.push_back("State.Stunned");
		ability->set_activation_blocked_tags(blocked);
		CHECK(ability->get_activation_blocked_tags().size() == 1);

		TypedArray<StringName> required;
		required.push_back("State.Grounded");
		ability->set_activation_required_tags(required);
		CHECK(ability->get_activation_required_tags().size() == 1);

		TypedArray<StringName> owned;
		owned.push_back("State.Dashing");
		ability->set_activation_owned_tags(owned);
		CHECK(ability->get_activation_owned_tags().size() == 1);
	}

	SUBCASE("Costs Management") {
		ability->add_cost("Stamina", 15.0f);
		CHECK(ability->get_cost_amount("Stamina") == 15.0f);
		CHECK(ability->get_costs().size() == 1);

		ability->remove_cost("Stamina");
		CHECK(ability->get_cost_amount("Stamina") == 0.0f);
		CHECK(ability->get_costs().size() == 0);
	}

	SUBCASE("Duration and Requirements") {
		ability->set_duration_policy(AbilitySystemAbility::POLICY_DURATION);
		CHECK(ability->get_duration_policy() == AbilitySystemAbility::POLICY_DURATION);

		ability->set_ability_duration(0.5f);
		CHECK(ability->get_ability_duration() == 0.5f);

		ability->add_requirement("Level", 5.0f);
		CHECK(ability->get_requirement_count() == 1);
		CHECK(ability->get_requirement_amount("Level") == 5.0f);

		ability->remove_requirement("Level");
		CHECK(ability->get_requirement_count() == 0);
	}
}
