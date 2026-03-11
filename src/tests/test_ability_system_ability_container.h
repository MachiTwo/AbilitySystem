/**************************************************************************/
/*  test_ability_system_ability_container.h                               */
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
#include "src/resources/ability_system_ability_container.h"
#include "src/resources/ability_system_attribute_set.h"
#include "src/resources/ability_system_effect.h"
#include "src/tests/doctest.h"
#include "src/tests/test_helpers.h"
#else
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_ability_container.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/tests/doctest.h"
#include "modules/ability_system/tests/test_helpers.h"
#endif

using namespace godot;

TEST_CASE("AbilitySystemAbilityContainer API") {
	Ref<AbilitySystemAbilityContainer> container = memnew(AbilitySystemAbilityContainer);

	SUBCASE("Abilities Management") {
		Ref<AbilitySystemAbility> ability = memnew(AbilitySystemAbility);
		ability->set_ability_name("Fireball");
		TypedArray<AbilitySystemAbility> abilities;
		abilities.push_back(ability);

		container->set_abilities(abilities);
		CHECK(container->get_abilities().size() == 1);
		CHECK(container->has_ability(ability) == true);

		Ref<AbilitySystemAbility> non_added = memnew(AbilitySystemAbility);
		CHECK(container->has_ability(non_added) == false);
	}

	SUBCASE("Effects Management") {
		Ref<AbilitySystemEffect> effect = memnew(AbilitySystemEffect);
		effect->set_effect_name("Buff");
		TypedArray<AbilitySystemEffect> effects;
		effects.push_back(effect);

		container->set_effects(effects);
		CHECK(container->get_effects().size() == 1);
		CHECK(container->has_effect(effect) == true);

		Ref<AbilitySystemEffect> non_added = memnew(AbilitySystemEffect);
		CHECK(container->has_effect(non_added) == false);
	}

	SUBCASE("Cues Management") {
		Ref<AbilitySystemCue> cue = memnew(AbilitySystemCue);
		cue->set_cue_tag("Cue.VFX.Explosion");
		TypedArray<AbilitySystemCue> cues;
		cues.push_back(cue);

		container->set_cues(cues);
		CHECK(container->get_cues().size() == 1);
		CHECK(container->has_cue("Cue.VFX.Explosion") == true);
		CHECK(container->has_cue("Cue.NonExistent") == false);
	}

	SUBCASE("Attribute Set Management") {
		Ref<AbilitySystemAttributeSet> attr_set = memnew(AbilitySystemAttributeSet);
		container->set_attribute_set(attr_set);
		CHECK(container->get_attribute_set() == attr_set);
	}
}
