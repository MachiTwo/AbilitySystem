/**************************************************************************/
/*  test_as_attribute_set.h                                               */
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

#ifndef TEST_AS_ATTRIBUTE_SET_H
#define TEST_AS_ATTRIBUTE_SET_H

#include "doctest.h"
#include "src/resources/as_attribute.h"
#include "src/resources/as_attribute_set.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AbilitySystem] ASAttributeSet (300% Coverage)") {
	Ref<ASAttributeSet> attr_set;
	attr_set.instantiate();

	SUBCASE("Attribute Definitions - 3 Variations") {
		Ref<ASAttribute> a1;
		a1.instantiate();
		a1->set_attribute_name("a1");
		Ref<ASAttribute> a2;
		a2.instantiate();
		a2->set_attribute_name("a2");
		Ref<ASAttribute> a3;
		a3.instantiate();
		a3->set_attribute_name("a3");

		// Var 1: Add
		attr_set->add_attribute_definition(a1);
		CHECK(attr_set->has_attribute("a1") == true);

		// Var 2: Remove
		attr_set->add_attribute_definition(a2);
		attr_set->remove_attribute_definition("a2");
		CHECK(attr_set->has_attribute("a2") == false);

		// Var 3: Bulk Set
		TypedArray<ASAttribute> defs;
		defs.push_back(a1);
		defs.push_back(a3);
		attr_set->set_attribute_definitions(defs);
		CHECK(attr_set->get_attribute_list().size() == 2);
	}

	SUBCASE("Value Clamping (Min/Max) - 3 Variations") {
		Ref<ASAttribute> hp;
		hp.instantiate();
		hp->set_attribute_name("hp");
		hp->set_min_value(0.0f);
		hp->set_max_value(100.0f);
		attr_set->add_attribute_definition(hp);

		// Var 1: High Clamp
		attr_set->set_attribute_base_value("hp", 150.0f);
		CHECK(attr_set->get_attribute_base_value("hp") == 100.0f);

		// Var 2: Low Clamp
		attr_set->set_attribute_base_value("hp", -50.0f);
		CHECK(attr_set->get_attribute_base_value("hp") == 0.0f);

		// Var 3: Inside Range
		attr_set->set_attribute_base_value("hp", 50.0f);
		CHECK(attr_set->get_attribute_base_value("hp") == 50.0f);
	}

	SUBCASE("Modifier Interaction - 3 Variations") {
		Ref<ASAttribute> stat;
		stat.instantiate();
		stat->set_attribute_name("stat");
		stat->set_base_value(10.0f);
		attr_set->add_attribute_definition(stat);

		// Var 1: Multiple Adds
		attr_set->add_modifier("stat", 5.0f); // 15
		attr_set->add_modifier("stat", 2.0f); // 17
		CHECK(attr_set->get_attribute_value("stat") == 17.0f);

		// Var 2: Mixed Mul/Add
		// Order: (Base + Adds) * Muls
		attr_set->add_modifier("stat", 2.0f, ASAttributeSet::MODIFIER_MULTIPLY); // 17 * 2 = 34
		CHECK(attr_set->get_attribute_value("stat") == 34.0f);

		// Var 3: Removal Recovery
		attr_set->remove_modifier("stat", 2.0f, ASAttributeSet::MODIFIER_MULTIPLY);
		CHECK(attr_set->get_attribute_value("stat") == 17.0f);
	}
}

#endif // TEST_AS_ATTRIBUTE_SET_H
