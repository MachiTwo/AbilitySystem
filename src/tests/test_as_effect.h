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

#ifndef TEST_AS_EFFECT_H
#define TEST_AS_EFFECT_H

#include "doctest.h"
#include "src/resources/as_effect.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AbilitySystem] ASEffect (300% Coverage)") {
	Ref<ASEffect> effect;
	effect.instantiate();

	SUBCASE("Duration Policies - 3 Variations") {
		// Var 1: Instant
		effect->set_duration_policy(ASEffect::POLICY_INSTANT);
		CHECK(effect->get_duration_policy() == ASEffect::POLICY_INSTANT);

		// Var 2: Duration
		effect->set_duration_policy(ASEffect::POLICY_DURATION);
		effect->set_duration_magnitude(5.0f);
		CHECK(effect->get_duration_magnitude() == 5.0f);

		// Var 3: Periodic
		effect->set_period(1.0f);
		CHECK(effect->get_period() == 1.0f);
	}

	SUBCASE("Modifiers - 3 Variations") {
		effect->add_modifier("str", ASEffect::OP_ADD, 10.0f);
		effect->add_modifier("dex", ASEffect::OP_MULTIPLY, 1.5f);
		effect->add_modifier("int", ASEffect::OP_OVERRIDE, 100.0f);

		// Var 1: Check Count
		CHECK(effect->get_modifier_count() == 3);

		// Var 2: Check Types
		CHECK(effect->get_modifier_operation(0) == ASEffect::OP_ADD);
		CHECK(effect->get_modifier_operation(1) == ASEffect::OP_MULTIPLY);
		CHECK(effect->get_modifier_operation(2) == ASEffect::OP_OVERRIDE);

		// Var 3: Check Magnitudes
		CHECK(effect->get_modifier_magnitude(0) == 10.0f);
		CHECK(effect->get_modifier_magnitude(1) == 1.5f);
		CHECK(effect->get_modifier_magnitude(2) == 100.0f);
	}

	SUBCASE("Stacking - 3 Variations") {
		// Var 1: Intensity
		effect->set_stacking_policy(ASEffect::STACK_INTENSITY);
		CHECK(effect->get_stacking_policy() == ASEffect::STACK_INTENSITY);

		// Var 2: Duration
		effect->set_stacking_policy(ASEffect::STACK_DURATION);
		CHECK(effect->get_stacking_policy() == ASEffect::STACK_DURATION);

		// Var 3: Override
		effect->set_stacking_policy(ASEffect::STACK_OVERRIDE);
		CHECK(effect->get_stacking_policy() == ASEffect::STACK_OVERRIDE);
	}
}

#endif // TEST_AS_EFFECT_H
