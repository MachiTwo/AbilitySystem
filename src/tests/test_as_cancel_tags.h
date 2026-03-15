/**************************************************************************/
/*  test_as_cancel_tags.h                                                 */
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
#include "src/resources/as_container.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/core/as_ability_spec.h"
#include "modules/ability_system/resources/as_ability.h"
#include "modules/ability_system/resources/as_container.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AS] Ability Cancel Tags") {
	ASComponent *asc = memnew(ASComponent);

	// Setup Abilities
	Ref<ASAbility> ability_idle = memnew(ASAbility);
	ability_idle->set_ability_tag("ability.idle");
	ability_idle->set_duration_policy(ASAbility::POLICY_INFINITE);

	Ref<ASAbility> ability_attack = memnew(ASAbility);
	ability_attack->set_ability_tag("ability.attack");
	ability_attack->set_duration_policy(ASAbility::POLICY_DURATION);
	ability_attack->set_ability_duration(1.0f);

	// Attack should cancel Idle
	TypedArray<StringName> cancel_tags;
	cancel_tags.push_back("ability.idle");
	ability_attack->set_activation_cancel_tags(cancel_tags);

	// Setup Container
	Ref<ASContainer> container = memnew(ASContainer);
	TypedArray<ASAbility> abilities;
	abilities.push_back(ability_idle);
	abilities.push_back(ability_attack);
	container->set_abilities(abilities);
	asc->set_container(container);

	// Unlock abilities
	asc->unlock_ability_by_tag("ability.idle");
	asc->unlock_ability_by_tag("ability.attack");

	SUBCASE("Activation Cancel Tags") {
		// 1. Activate Idle
		MESSAGE("Activating Idle...");
		CHECK(asc->try_activate_ability_by_tag("ability.idle") == true);
		CHECK(asc->is_ability_active("ability.idle") == true);
		CHECK(asc->get_active_abilities().size() == 1);

		// 2. Activate Attack (should cancel Idle)
		MESSAGE("Activating Attack (should cancel Idle)...");
		CHECK(asc->try_activate_ability_by_tag("ability.attack") == true);

		// 3. Verify Idle is cancelled
		CHECK(asc->is_ability_active("ability.idle") == false);
		CHECK(asc->is_ability_active("ability.attack") == true);
		CHECK(asc->get_active_abilities().size() == 1);
	}

	memdelete(asc);
}
