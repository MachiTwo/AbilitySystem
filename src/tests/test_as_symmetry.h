/**************************************************************************/
/*  test_as_symmetry.h                                                    */
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

#include "test_helpers.h"

TEST_CASE("Ability System API Symmetry") {
	ASComponent *asc = make_standard_asc();
	const StringName tag_spell = "Ability.Fireball";

	// Setup Resource
	Ref<ASAbility> spell = make_ability("Fireball", tag_spell, nullptr, 50.0f, "Mana");
	spell->set_ability_duration(3.5f);
	spell->set_duration_policy(ASAbility::POLICY_DURATION);
	spell->set_cooldown_duration(10.0f);

	// Setup Container
	Ref<ASContainer> catalog = memnew(ASContainer);
	TypedArray<ASAbility> abilities;
	abilities.push_back(spell);
	catalog->set_abilities(abilities);
	asc->set_container(catalog);
	asc->unlock_ability_by_resource(spell);

	SUBCASE("Layer 1: Component Preview (Pre-activation)") {
		CHECK(asc->get_ability_duration_preview(tag_spell) == 3.5f);
		CHECK(asc->get_ability_cost_preview(tag_spell, "Mana") == 50.0f);
		CHECK(asc->get_ability_cooldown_preview(tag_spell) == 10.0f);
	}

	SUBCASE("Layer 2: Spec Data (Post-activation)") {
		asc->try_activate_ability_by_tag(tag_spell);
		TypedArray<ASAbilitySpec> active = asc->get_active_abilities_debug();
		CHECK(active.size() == 1);
		if (active.size() > 0) {
			Ref<ASAbilitySpec> spec = active[0];

			CHECK(spec->get_total_duration() == 3.5f);
			CHECK(spec->get_cost_amount("Mana") == 50.0f);
			CHECK(spec->get_cooldown_duration() == 10.0f);
		}
	}

	memdelete(asc);
}
