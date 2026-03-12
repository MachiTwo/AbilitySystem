/**************************************************************************/
/*  test_as_coverage_gap.h                                                */
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
#include "test_signal_watcher.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("Ability System Coverage Gaps") {
	ASComponent *asc = make_standard_asc();

	SUBCASE("Node Registration API") {
		Node *dummy_node = memnew(Node);
		const StringName alias = "TestRenderer";

		asc->register_node(alias, dummy_node);
		CHECK(asc->get_node_ptr(alias) == dummy_node);

		asc->unregister_node(alias);
		CHECK(asc->get_node_ptr(alias) == nullptr);

		memdelete(dummy_node);
	}

	SUBCASE("Effect Stacking: STACK_NEW_INSTANCE") {
		Ref<ASEffect> effect = create_test_effect("IndependentBuff", ASEffect::POLICY_INFINITE);
		effect->set_stacking_policy(ASEffect::STACK_NEW_INSTANCE);

		asc->apply_effect_by_resource(effect);
		asc->apply_effect_by_resource(effect);

		CHECK(asc->get_active_effects_debug().size() == 2);
	}

	SUBCASE("Periodic Effect Ticks") {
		Ref<ASEffect> poison = create_test_effect("Poison", ASEffect::POLICY_DURATION, 5.0f);
		poison->set_period(1.0f);
		poison->add_modifier("Health", ASEffect::OP_ADD, -10.0f);

		asc->apply_effect_by_resource(poison); // Initial health 100
		CHECK_ATTR_EQ(asc, "Health", 100.0f); // Instant application is FALSE by default

		asc->tick(0.5f);
		CHECK_ATTR_EQ(asc, "Health", 100.0f);

		asc->tick(0.6f); // Total 1.1s -> should have ticked once
		CHECK_ATTR_EQ(asc, "Health", 90.0f);

		asc->tick(1.0f); // Total 2.1s -> second tick
		CHECK_ATTR_EQ(asc, "Health", 80.0f);
	}

	SUBCASE("Null Safety and Empty Tags") {
		// These should fail gracefully via ERR_FAIL macros without crashing
		asc->apply_effect_by_resource(nullptr);
		asc->unlock_ability_by_resource(nullptr);

		CHECK(asc->get_active_effects_debug().size() == 0);
		CHECK(asc->get_unlocked_abilities_debug().size() == 0);

		// Non-existent tags
		CHECK(asc->has_tag("") == false);
		CHECK(asc->get_attribute_value_by_tag("InvalidAttribute") == 0.0f);
	}

	SUBCASE("Signal Emission: ability_failed") {
		Ref<ASTestSignalWatcher> watcher = memnew(ASTestSignalWatcher);
		asc->connect("ability_failed", Callable(watcher.ptr(), "_on_signal_received"));

		// Attempt to activate ability without unlocking or catalog
		asc->try_activate_ability_by_tag("NonExistent");

		CHECK(watcher->signal_count == 1);
		CHECK(watcher->last_reason.contains("not in catalog"));
	}

	memdelete(asc);
}
