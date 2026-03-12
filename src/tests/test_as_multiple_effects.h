/**************************************************************************/
/*  test_as_multiple_effects.h                                            */
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
#include "src/resources/as_effect.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#include "src/tests/test_helpers.h"
#include "src/tests/test_signal_watcher.h"
#else
#include "modules/ability_system/core/as_ability_spec.h"
#include "modules/ability_system/resources/as_ability.h"
#include "modules/ability_system/resources/as_effect.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#include "modules/ability_system/tests/test_helpers.h"
#include "modules/ability_system/tests/test_signal_watcher.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("AbilitySystem Multiple Effects and Target Filtering") {
	ASComponent *asc = make_standard_asc();

	Ref<ASEffect> self_effect = memnew(ASEffect);
	self_effect->set_effect_name("SelfEffect");
	self_effect->set_target_type(ASEffect::TARGET_SELF);
	self_effect->set_duration_policy(ASEffect::POLICY_INSTANT);
	self_effect->add_modifier("Health", ASEffect::OP_ADD, 10.0f);

	Ref<ASEffect> others_effect = memnew(ASEffect);
	others_effect->set_effect_name("OthersEffect");
	others_effect->set_target_type(ASEffect::TARGET_OTHERS);
	others_effect->set_duration_policy(ASEffect::POLICY_INSTANT);
	others_effect->add_modifier("Health", ASEffect::OP_ADD, -20.0f);

	Ref<ASAbility> ability = memnew(ASAbility);
	ability->set_ability_tag("Ability.MultiEffect");

	TypedArray<ASEffect> effects;
	effects.push_back(self_effect);
	effects.push_back(others_effect);
	ability->set_effects(effects);

	SUBCASE("ASAbilitySpec target filtering") {
		Ref<ASAbilitySpec> spec = memnew(ASAbilitySpec);
		spec->init(ability, 1);

		TypedArray<ASEffect> self_results = spec->get_effects_for_target(ASEffect::TARGET_SELF);
		CHECK(self_results.size() == 1);
		CHECK(self_results[0] == self_effect);

		TypedArray<ASEffect> others_results = spec->get_effects_for_target(ASEffect::TARGET_OTHERS);
		CHECK(others_results.size() == 1);
		CHECK(others_results[0] == others_effect);
	}

	SUBCASE("Signal Emission on Activation") {
		Ref<ASTestSignalWatcher> watcher_self = memnew(ASTestSignalWatcher);
		Ref<ASTestSignalWatcher> watcher_others = memnew(ASTestSignalWatcher);

		asc->connect("effects_applied_to_self", Callable(watcher_self.ptr(), "_on_signal_received"));
		asc->connect("effects_ready_for_others", Callable(watcher_others.ptr(), "_on_signal_received"));

		// Setup catalog and unlock
		Ref<ASContainer> catalog = memnew(ASContainer);
		TypedArray<ASAbility> catalog_abilities;
		catalog_abilities.push_back(ability);
		catalog->set_abilities(catalog_abilities);
		asc->set_container(catalog);
		asc->unlock_ability_by_resource(ability);

		// Initial health is 100 (from make_standard_asc default)
		CHECK_ATTR_EQ(asc, "Health", 100.0f);

		bool activated = asc->try_activate_ability_by_tag("Ability.MultiEffect");
		CHECK(activated == true);

		// Self effect should be applied automatically (+10)
		CHECK_ATTR_EQ(asc, "Health", 110.0f);

		// Signals should be emitted
		CHECK(watcher_self->signal_count == 1);
		CHECK(watcher_others->signal_count == 1);
	}

	memdelete(asc);
}
