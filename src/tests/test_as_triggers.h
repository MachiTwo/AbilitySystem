/**************************************************************************/
/*  test_as_triggers.h                                                    */
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
#include "src/core/as_ability_spec.h"
#include "src/resources/as_ability.h"
#include "src/resources/as_attribute_set.h"
#include "src/resources/as_container.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/core/as_ability_spec.h"
#include "modules/ability_system/resources/as_ability.h"
#include "modules/ability_system/resources/as_attribute_set.h"
#include "modules/ability_system/resources/as_container.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AS] Rigid Ability Triggers & Integration") {
	ASComponent *asc = memnew(ASComponent);

	// Setup Attributes for requirement testing
	Ref<ASAttributeSet> attr_set;
	attr_set.instantiate();

	Ref<ASAttribute> attr_mana;
	attr_mana.instantiate();
	attr_mana->set_attribute_name("Mana");
	attr_mana->set_min_value(0.0f);
	attr_mana->set_max_value(1000.0f);

	attr_set->add_attribute_definition(attr_mana);
	attr_set->set_attribute_base_value("Mana", 100.0f);
	asc->add_attribute_set(attr_set);

	// Setup Ability 1: Reaction (Duration based)
	Ref<ASAbility> ability_reaction;
	ability_reaction.instantiate();
	ability_reaction->set_ability_tag("ability.reaction");
	ability_reaction->set_duration_policy(ASAbility::POLICY_DURATION);
	ability_reaction->set_ability_duration(5.0f);
	ability_reaction->add_trigger("state.triggered", ASAbility::TRIGGER_ON_TAG_ADDED);

	// Setup Ability 2: Counter (Instant, costs Mana)
	Ref<ASAbility> ability_counter;
	ability_counter.instantiate();
	ability_counter->set_ability_tag("ability.counter");
	ability_counter->set_duration_policy(ASAbility::POLICY_INSTANT);
	ability_counter->add_trigger("state.attacked", ASAbility::TRIGGER_ON_TAG_ADDED);
	ability_counter->add_cost("Mana", 30.0f);

	// Setup Ability 3: Heavy Strike (Trigger on tag REMOVED)
	Ref<ASAbility> ability_heavy;
	ability_heavy.instantiate();
	ability_heavy->set_ability_tag("ability.heavy_strike");
	ability_heavy->set_duration_policy(ASAbility::POLICY_DURATION);
	ability_heavy->set_ability_duration(1.0f);
	ability_heavy->add_trigger("state.blocking", ASAbility::TRIGGER_ON_TAG_REMOVED);

	// Apply via Container to test full pipeline
	Ref<ASContainer> container;
	container.instantiate();
	TypedArray<ASAbility> abilities;
	abilities.push_back(ability_reaction);
	abilities.push_back(ability_counter);
	abilities.push_back(ability_heavy);
	container->set_abilities(abilities);

	asc->apply_container(container);

	// Unlocking them to make them eligible for triggers
	asc->unlock_ability_by_tag("ability.reaction");
	asc->unlock_ability_by_tag("ability.counter");
	asc->unlock_ability_by_tag("ability.heavy_strike");

	SUBCASE("Rigid: Trigger on Tag Added (Success)") {
		CHECK(asc->get_active_abilities().size() == 0);
		asc->add_tag("state.triggered");

		int active_count = asc->get_active_abilities().size();
		CHECK(active_count == 1);
		if (active_count == 1) {
			Ref<ASAbilitySpec> spec = asc->get_active_abilities()[0];
			CHECK(spec.is_valid());
			if (spec.is_valid()) {
				CHECK(spec->get_ability()->get_ability_tag() == StringName("ability.reaction"));
				CHECK(spec->get_is_active() == true);
			}
		}
	}

	SUBCASE("Rigid: Trigger with Costs (Mana Check)") {
		asc->cancel_all_abilities();
		CHECK(asc->get_attribute_value_by_tag("Mana") == 100.0f);

		// Trigger 1: Counter (Costs 30)
		asc->add_tag("state.attacked");
		CHECK(asc->get_attribute_value_by_tag("Mana") == 70.0f);

		// Instant abilities don't stay in active_abilities
		CHECK(asc->get_active_abilities().size() == 0);

		// Trigger 2: Counter again (Costs 30)
		asc->remove_tag("state.attacked");
		asc->add_tag("state.attacked");
		CHECK(asc->get_attribute_value_by_tag("Mana") == 40.0f);

		// Trigger 3: Counter again (Costs 30)
		asc->remove_tag("state.attacked");
		asc->add_tag("state.attacked");
		CHECK(asc->get_attribute_value_by_tag("Mana") == 10.0f);

		// Trigger 4: Should FAIL (Not enough mana)
		asc->remove_tag("state.attacked");
		asc->add_tag("state.attacked");
		CHECK(asc->get_attribute_value_by_tag("Mana") == 10.0f); // Mana remains same
	}

	SUBCASE("Rigid: Trigger on Tag Removed") {
		asc->cancel_all_abilities();

		asc->add_tag("state.blocking");
		CHECK(asc->get_active_abilities().size() == 0);

		// Removing the tag should trigger Heavy Strike
		asc->remove_tag("state.blocking");
		CHECK(asc->get_active_abilities().size() == 1);
		if (asc->get_active_abilities().size() == 1) {
			Ref<ASAbilitySpec> heavy_spec = asc->get_active_abilities()[0];
			CHECK(heavy_spec.is_valid());
			if (heavy_spec.is_valid()) {
				CHECK(heavy_spec->get_ability()->get_ability_tag() == StringName("ability.heavy_strike"));
			}
		}
	}

	SUBCASE("Rigid: Blocked Triggers") {
		asc->cancel_all_abilities();

		// We must find the duplicated ability in the component's catalog
		Ref<ASAbility> local_ability;
		TypedArray<ASAbility> catalog = asc->get_container()->get_abilities();
		for (int i = 0; i < catalog.size(); i++) {
			Ref<ASAbility> a = catalog[i];
			if (a.is_valid() && a->get_ability_tag() == StringName("ability.reaction")) {
				local_ability = a;
				break;
			}
		}
		CHECK(local_ability.is_valid());

		// Add a blocking tag to the ability
		if (local_ability.is_valid()) {
			TypedArray<StringName> blocked;
			blocked.push_back(StringName("State.Stunned"));
			local_ability->set_activation_blocked_tags(blocked);
		}

		// Player is stunned
		asc->add_tag("State.Stunned");

		// Try to trigger
		asc->add_tag("state.triggered");

		// Should NOT activate because of Stunned tag
		CHECK(asc->get_active_abilities().size() == 0);
	}

	memdelete(asc);
}
