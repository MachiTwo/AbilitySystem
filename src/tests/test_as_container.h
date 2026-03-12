/**************************************************************************/
/*  test_as_container.h                                                   */
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
#include "src/resources/as_ability.h"
#include "src/resources/as_attribute_set.h"
#include "src/resources/as_container.h"
#include "src/resources/as_effect.h"
#include "src/tests/doctest.h"
#include "src/tests/test_helpers.h"
#else
#include "modules/ability_system/resources/as_ability.h"
#include "modules/ability_system/resources/as_attribute_set.h"
#include "modules/ability_system/resources/as_container.h"
#include "modules/ability_system/resources/as_effect.h"
#include "modules/ability_system/tests/doctest.h"
#include "modules/ability_system/tests/test_helpers.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("ASContainer API") {
	Ref<ASContainer> container = memnew(ASContainer);

	SUBCASE("Abilities Management") {
		Ref<ASAbility> ability = memnew(ASAbility);
		ability->set_ability_name("Fireball");
		TypedArray<ASAbility> abilities;
		abilities.push_back(ability);

		container->set_abilities(abilities);
		CHECK(container->get_abilities().size() == 1);
		CHECK(container->has_ability(ability) == true);

		Ref<ASAbility> non_added = memnew(ASAbility);
		CHECK(container->has_ability(non_added) == false);
	}

	SUBCASE("Effects Management") {
		Ref<ASEffect> effect = memnew(ASEffect);
		effect->set_effect_name("Buff");
		TypedArray<ASEffect> effects;
		effects.push_back(effect);

		container->set_effects(effects);
		CHECK(container->get_effects().size() == 1);
		CHECK(container->has_effect(effect) == true);

		Ref<ASEffect> non_added = memnew(ASEffect);
		CHECK(container->has_effect(non_added) == false);
	}

	SUBCASE("Cues Management") {
		Ref<ASCue> cue = memnew(ASCue);
		cue->set_cue_tag("Cue.VFX.Explosion");
		TypedArray<ASCue> cues;
		cues.push_back(cue);

		container->set_cues(cues);
		CHECK(container->get_cues().size() == 1);
		CHECK(container->has_cue("Cue.VFX.Explosion") == true);
		CHECK(container->has_cue("Cue.NonExistent") == false);
	}

	SUBCASE("Attribute Set Management") {
		Ref<ASAttributeSet> attr_set = memnew(ASAttributeSet);
		container->set_attribute_set(attr_set);
		CHECK(container->get_attribute_set() == attr_set);
	}
}
