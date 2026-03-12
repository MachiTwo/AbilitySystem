/**************************************************************************/
/*  test_ability_system.h                                                 */
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
#include "src/core/ability_system.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("AbilitySystem Tag Matching") {
	SUBCASE("Exact matches") {
		CHECK(AbilitySystem::tag_matches("State.Dead", "State.Dead", true) == true);
		CHECK(AbilitySystem::tag_matches("State.Dead", "State.Alive", true) == false);
		CHECK(AbilitySystem::tag_matches("State", "State", true) == true);
	}

	SUBCASE("Hierarchical matches") {
		CHECK(AbilitySystem::tag_matches("State.Dead", "State", false) == true);
		CHECK(AbilitySystem::tag_matches("State.Dead.Bleeding", "State", false) == true);
		CHECK(AbilitySystem::tag_matches("State.Dead.Bleeding", "State.Dead", false) == true);

		CHECK(AbilitySystem::tag_matches("State", "State.Dead", false) == false);
		CHECK(AbilitySystem::tag_matches("States", "State", false) == false);
		CHECK(AbilitySystem::tag_matches("State.Dead", "Sta", false) == false);
	}

	SUBCASE("Partial string matches without dot separator") {
		// "Apple" should not match "App"
		CHECK(AbilitySystem::tag_matches("Apple", "App", false) == false);
		// "App.Apple" should match "App"
		CHECK(AbilitySystem::tag_matches("App.Apple", "App", false) == true);
	}
}

TEST_CASE("AbilitySystem Singleton Registry") {
	AbilitySystem *as = AbilitySystem::get_singleton();

	SUBCASE("Register and Unregister Tags") {
		as->register_tag("Test.Tag.A", AbilitySystem::TAG_TYPE_NAME);
		CHECK(as->is_tag_registered("Test.Tag.A") == true);
		CHECK(as->get_tag_type("Test.Tag.A") == AbilitySystem::TAG_TYPE_NAME);

		as->register_tag("Test.Tag.B", AbilitySystem::TAG_TYPE_CONDITIONAL);
		CHECK(as->is_tag_registered("Test.Tag.B") == true);
		CHECK(as->get_tag_type("Test.Tag.B") == AbilitySystem::TAG_TYPE_CONDITIONAL);

		TypedArray<StringName> all_tags = as->get_registered_tags();
		bool found_a = false;
		for (int i = 0; i < all_tags.size(); i++) {
			if (StringName(all_tags[i]) == StringName("Test.Tag.A")) {
				found_a = true;
				break;
			}
		}
		CHECK(found_a == true);

		TypedArray<StringName> conditional_tags = as->get_registered_tags_of_type(AbilitySystem::TAG_TYPE_CONDITIONAL);
		bool found_b = false;
		for (int i = 0; i < conditional_tags.size(); i++) {
			if (StringName(conditional_tags[i]) == StringName("Test.Tag.B")) {
				found_b = true;
				break;
			}
		}
		CHECK(found_b == true);

		as->unregister_tag("Test.Tag.A");
		CHECK(as->is_tag_registered("Test.Tag.A") == false);
		as->unregister_tag("Test.Tag.B");
		CHECK(as->is_tag_registered("Test.Tag.B") == false);
	}

	SUBCASE("Resource Names Management") {
		String test_name = "UniqueResourceName";
		uint64_t owner_id = 12345;

		CHECK(as->register_resource_name(test_name, owner_id) == true);
		CHECK(as->get_resource_name_owner(test_name) == owner_id);

		// Re-registering with different owner should fail or override?
		// According to common registry patterns, let's check return.
		CHECK(as->register_resource_name(test_name, 54321) == false);
		CHECK(as->get_resource_name_owner(test_name) == owner_id);

		as->unregister_resource_name(test_name);
		CHECK(as->get_resource_name_owner(test_name) == 0);
	}
}
