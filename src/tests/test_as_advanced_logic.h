/**************************************************************************/
/*  test_as_advanced_logic.h                                              */
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
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#include "src/tests/test_helpers.h"
#else
#include "modules/ability_system/resources/as_effect.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#include "modules/ability_system/tests/test_helpers.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("AbilitySystem Advanced Logic (Periods & Modifiers)") {
	SUBCASE("Periodic Effect (DoT)") {
		ASComponent *asc = make_standard_asc();
		Ref<ASEffect> dot = memnew(ASEffect);
		dot->set_effect_name("Burn");
		dot->set_duration_policy(ASEffect::POLICY_DURATION);
		dot->set_duration_magnitude(2.5f);
		dot->set_period(1.0f);
		dot->set_execute_periodic_tick_on_application(true);
		dot->add_modifier("Health", ASEffect::OP_ADD, -10.0f);

		// Initial HP 100
		CHECK_ATTR_EQ(asc, "Health", 100.0f);

		asc->apply_effect_by_resource(dot);

		// Immediate tick on application
		CHECK_ATTR_EQ(asc, "Health", 90.0f);

		// Process 1.1s (should trigger second tick)
		asc->tick(1.1f);
		CHECK_ATTR_EQ(asc, "Health", 80.0f);

		// Process 1.0s more (should trigger third tick)
		asc->tick(1.0f);
		CHECK_ATTR_EQ(asc, "Health", 70.0f);

		asc->tick(1.0f);
		CHECK(asc->has_active_effect_by_resource(dot) == false);
		CHECK_ATTR_EQ(asc, "Health", 70.0f);
		memdelete(asc);
	}

	SUBCASE("Modifier Multiplication") {
		ASComponent *asc = make_standard_asc();
		asc->set_attribute_base_value_by_tag("Health", 100.0f);

		Ref<ASEffect> buff = memnew(ASEffect);
		buff->add_modifier("Health", ASEffect::OP_MULTIPLY, 2.0f);
		buff->set_duration_policy(ASEffect::POLICY_DURATION);
		buff->set_duration_magnitude(1.0f);

		asc->apply_effect_by_resource(buff);
		CHECK_ATTR_EQ(asc, "Health", 200.0f);

		asc->tick(1.1f);
		CHECK_ATTR_EQ(asc, "Health", 100.0f);
		memdelete(asc);
	}

	SUBCASE("Effect Blocking by Tags") {
		ASComponent *asc = make_standard_asc();
		// Standard ASC has 100 HP
		Ref<ASEffect> damage = memnew(ASEffect);
		damage->add_modifier("Health", ASEffect::OP_ADD, -10.0f);

		TypedArray<StringName> blocked;
		blocked.push_back("state.immune");
		damage->set_activation_blocked_any_tags(blocked);

		// 1. Apply without tag
		CHECK(asc->can_activate_effect_by_resource(damage) == true);
		asc->apply_effect_by_resource(damage);
		CHECK_ATTR_EQ(asc, "Health", 90.0f);

		// 2. Add tag and try again
		asc->add_tag("state.immune");
		CHECK(asc->can_activate_effect_by_resource(damage) == false);

		// Signal failure check? (Manually applied doesn't always emit if blocked at can_activate level)
		asc->apply_effect_by_resource(damage);
		CHECK_ATTR_EQ(asc, "Health", 90.0f); // Should NOT have changed
		memdelete(asc);
	}
}
