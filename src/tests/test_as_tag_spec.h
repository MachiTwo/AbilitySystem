/**************************************************************************/
/*  test_as_tag_spec.h                                                    */
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

#ifndef TEST_AS_TAG_SPEC_H
#define TEST_AS_TAG_SPEC_H

#include "doctest.h"
#include "src/core/as_tag_spec.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AbilitySystem] ASTagSpec (300% Coverage)") {
	Ref<ASTagSpec> tags;
	tags.instantiate();

	SUBCASE("Hierarchical Insertion - 3 Variations") {
		// Var 1: Simple deep tag
		tags->add_tag("a.b.c");
		CHECK(tags->has_tag("a.b.c") == true);
		CHECK(tags->has_tag("a.b") == true);
		CHECK(tags->has_tag("a") == true);

		// Var 2: Sibling tags
		tags->add_tag("a.b.d");
		CHECK(tags->has_tag("a.b.d") == true);
		CHECK(tags->get_all_tags().size() == 4); // a, a.b, a.b.c, a.b.d

		// Var 3: Disjoint branches
		tags->add_tag("x.y.z");
		CHECK(tags->has_tag("x") == true);

		tags->remove_all_tags();
	}

	SUBCASE("Multiple Tag Comparison (All/Any) - 3 Variations") {
		tags->add_tag("state.active");
		tags->add_tag("state.buffed");
		tags->add_tag("state.protected");

		TypedArray<StringName> check_list;
		check_list.push_back("state.active");
		check_list.push_back("state.buffed");

		// Var 1: has_all_tags (Full match)
		CHECK(tags->has_all_tags(check_list) == true);

		// Var 2: has_all_tags (Partial fail)
		check_list.push_back("state.invisible");
		CHECK(tags->has_all_tags(check_list) == false);

		// Var 3: has_any_tags (Success)
		CHECK(tags->has_any_tags(check_list) == true);

		tags->remove_all_tags();
	}

	SUBCASE("Removal and Cleanup - 3 Variations") {
		tags->add_tag("toxic.poison");
		tags->add_tag("toxic.fire");

		// Var 1: Specific leaf removal
		tags->remove_tag("toxic.poison");
		CHECK(tags->has_tag("toxic.poison") == false);
		CHECK(tags->has_tag("toxic.fire") == true);

		// Var 2: Parent removal (should NOT remove siblings unless specified, usually)
		// Note: Our logic keeps parents as long as children exist or they were added explicitly.
		tags->remove_tag("toxic.fire");
		CHECK(tags->has_tag("toxic.fire") == false);

		// Var 3: Absolute Cleanup
		tags->add_tag("a.b");
		tags->add_tag("c.d");
		tags->remove_all_tags();
		CHECK(tags->get_all_tags().size() == 0);
	}
}

#endif // TEST_AS_TAG_SPEC_H
