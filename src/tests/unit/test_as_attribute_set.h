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

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/core/ability_system.h"
#include "src/resources/as_attribute.h"
#include "src/resources/as_attribute_set.h"
#include "src/tests/doctest.h"
#include "src/tests/test_tools.h"
#else
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/resources/as_attribute.h"
#include "modules/ability_system/resources/as_attribute_set.h"
#include "modules/ability_system/tests/doctest.h"
#include "modules/ability_system/tests/test_tools.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[ASAttributeSet] Core Value and Bounds Logic") {
	Ref<ASAttributeSet> attr_set;
	attr_set.instantiate();

	Ref<ASAttribute> hp_attr;
	hp_attr.instantiate();
	hp_attr->set_attribute_name(StringName("Health"));
	hp_attr->set_min_value(0.0f);
	hp_attr->set_max_value(100.0f);
	hp_attr->set_default_value(50.0f);

	attr_set->add_attribute_definition(hp_attr);

	SUBCASE("Base Value Assignment - 3 Variations") {
		// Var 1: Happy path - assigned value within bounds
		attr_set->set_attribute_base_value("Health", 75.0f);
		CHECK(attr_set->get_attribute_base_value("Health") == doctest::Approx(75.0f));

		// Var 2: Edge case bounds - clamping at max limit
		attr_set->set_attribute_base_value("Health", 150.0f);
		bool is_clamped = attr_set->get_attribute_base_value("Health") <= 100.0f;
		CHECK_MESSAGE(is_clamped, "Value should be clamped to max_value (100.0).");

		// Var 3: Negative - Querying an unregistered attribute
		ability_system::tests::ErrorDetector err_detector;
		err_detector.clear();
		float invalid_val = attr_set->get_attribute_base_value("Mana");
		CHECK_MESSAGE(invalid_val == doctest::Approx(0.0f), "Querying missing attribute should gracefully return 0.0f baseline.");
#ifndef ABILITY_SYSTEM_GDEXTENSION
		CHECK_MESSAGE(err_detector.has_error, "Querying missing attribute should log a push_error.");
#endif
	}

	SUBCASE("Modifiers System - 3 Variations") {
		// Preset Base to 50
		attr_set->set_attribute_base_value("Health", 50.0f);

		// Var 1: Happy path - Flat Add modifier
		attr_set->add_modifier("Health", 20.0f, ASAttributeSet::MODIFIER_ADD);
		CHECK(attr_set->get_attribute_value("Health") == doctest::Approx(70.0f));

		// Var 2: Edge case bounds - Multiplying past the max limit ceiling
		attr_set->add_modifier("Health", 2.0f, ASAttributeSet::MODIFIER_MULTIPLY);
		// Base 50 * 2 = 100 + 20 flat = 120. Should clamp at 100!
		bool is_modifier_clamped = attr_set->get_attribute_value("Health") <= 100.0f;
		CHECK_MESSAGE(is_modifier_clamped, "Modifiers resulting over max should be clamped upon resolution.");

		// Var 3: Negative - Negative multiplication
		// Base 50 * -1 = -50 + 20 = -30. Should clamp at 0!
		attr_set->remove_modifier("Health", 2.0f, ASAttributeSet::MODIFIER_MULTIPLY); // Remove previous
		attr_set->add_modifier("Health", -1.0f, ASAttributeSet::MODIFIER_MULTIPLY);
		CHECK_MESSAGE(attr_set->get_attribute_value("Health") >= 0.0f, "Modifiers resulting below 0 should clamp to min.");
	}
}

#endif // TEST_AS_ATTRIBUTE_SET_H
