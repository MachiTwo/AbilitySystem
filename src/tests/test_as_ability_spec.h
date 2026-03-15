/**************************************************************************/
/*  test_as_ability_spec.h                                                */
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

#ifndef TEST_AS_ABILITY_SPEC_H
#define TEST_AS_ABILITY_SPEC_H

#include "doctest.h"
#include "src/core/as_ability_spec.h"
#include "src/resources/as_ability.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AbilitySystem] ASAbilitySpec (300% Coverage)") {
	Ref<ASAbility> ability;
	ability.instantiate();

	Ref<ASAbilitySpec> spec;
	spec.instantiate();

	SUBCASE("Initialization - 3 Variations") {
		// Var 1: Default
		spec->init(ability);
		CHECK(spec->get_level() == 1.0f);

		// Var 2: Custom Level
		spec->init(ability, 5.0f);
		CHECK(spec->get_level() == 5.0f);

		// Var 3: Re-init
		Ref<ASAbility> ability2;
		ability2.instantiate();
		spec->init(ability2, 10.0f);
		CHECK(spec->get_ability() == ability2);
		CHECK(spec->get_level() == 10.0f);
	}

	SUBCASE("State Tracking - 3 Variations") {
		spec->init(ability);

		// Var 1: Active
		spec->set_is_active(true);
		CHECK(spec->get_is_active() == true);

		// Var 2: Inactive
		spec->set_is_active(false);
		CHECK(spec->get_is_active() == false);

		// Var 3: Target Node Persistence
		Node *target = memnew(Node);
		spec->set_target_node(target);
		CHECK(spec->get_target_node() == target);
		memdelete(target);
	}

	SUBCASE("Ticking & Duration - 3 Variations") {
		ability->set_duration_policy(ASAbility::POLICY_DURATION);
		ability->set_ability_duration(1.0f);
		spec->init(ability);
		spec->activate(nullptr); // Usually sets duration remaining

		// Var 1: Partial Tick
		CHECK(spec->tick(0.4f) == false); // Not finished

		// Var 2: Multiple Ticks
		CHECK(spec->tick(0.4f) == false); // Total 0.8, not finished

		// Var 3: Final Tick
		CHECK(spec->tick(0.3f) == true); // Total 1.1, finished
	}
}

#endif // TEST_AS_ABILITY_SPEC_H
