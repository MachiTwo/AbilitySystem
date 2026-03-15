/**************************************************************************/
/*  test_as_attribute_drivers.h                                           */
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

#ifndef TEST_AS_ATTRIBUTE_DRIVERS_H
#define TEST_AS_ATTRIBUTE_DRIVERS_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/tests/test_helpers.h"
#else
#include "modules/ability_system/tests/test_helpers.h"
#endif

TEST_CASE("[AbilitySystem] Attribute Drivers (300% Coverage)") {
	ASComponent *component = memnew(ASComponent);
	Ref<ASAttributeSet> set;
	set.instantiate();

	set->add_attribute_definition(create_test_attribute("stat.str", 10.0f));
	set->add_attribute_definition(create_test_attribute("stat.dex", 10.0f));
	set->add_attribute_definition(create_test_attribute("stat.int", 10.0f));
	set->add_attribute_definition(create_test_attribute("stat.atk", 0.0f));
	set->add_attribute_definition(create_test_attribute("stat.spd", 0.0f));
	set->add_attribute_definition(create_test_attribute("stat.mana", 0.0f));

	component->add_attribute_set(set);

	SUBCASE("Relationship Ratios - 3 Variations") {
		TypedArray<Dictionary> drivers;

		Dictionary d1;
		d1["source"] = StringName("stat.str");
		d1["destination"] = StringName("stat.atk");
		d1["ratio"] = 2.0f;
		drivers.push_back(d1);
		set->set_attribute_drivers(drivers);

		// Var 1: Base Ratio (10 * 2 = 20)
		CHECK_ATTR_EQ(component, "stat.atk", 20.0f);

		// Var 2: Dynamic Update (20 * 2 = 40)
		component->set_attribute_base_value_by_tag("stat.str", 20.0f);
		CHECK_ATTR_EQ(component, "stat.atk", 40.0f);

		// Var 3: Inverse/Zero Ratio
		d1["ratio"] = 0.0f;
		drivers.clear();
		drivers.push_back(d1);
		set->set_attribute_drivers(drivers);
		CHECK_ATTR_EQ(component, "stat.atk", 0.0f);
	}

	SUBCASE("Multiple Sources (Summation) - 3 Variations") {
		TypedArray<Dictionary> drivers;
		// ATK = STR * 2 + DEX * 1
		Dictionary d1;
		d1["source"] = StringName("stat.str");
		d1["destination"] = StringName("stat.atk");
		d1["ratio"] = 2.0f;
		Dictionary d2;
		d2["source"] = StringName("stat.dex");
		d2["destination"] = StringName("stat.atk");
		d2["ratio"] = 1.0f;

		drivers.push_back(d1);
		drivers.push_back(d2);
		set->set_attribute_drivers(drivers);

		// Reset values
		component->set_attribute_base_value_by_tag("stat.str", 10.0f);
		component->set_attribute_base_value_by_tag("stat.dex", 5.0f);

		// Var 1: Initial Sum (10*2 + 5*1 = 25)
		CHECK_ATTR_EQ(component, "stat.atk", 25.0f);

		// Var 2: Incremental Change (STR 10->15 : 15*2 + 5*1 = 35)
		component->set_attribute_base_value_by_tag("stat.str", 15.0f);
		CHECK_ATTR_EQ(component, "stat.atk", 35.0f);

		// Var 3: Removing a Driver (Sum should drop)
		drivers.remove_at(1); // Remove DEX driver
		set->set_attribute_drivers(drivers); // Recalculate: 15*2 = 30
		CHECK_ATTR_EQ(component, "stat.atk", 30.0f);
	}

	SUBCASE("Driver + Modifier Consistency - 3 Variations") {
		TypedArray<Dictionary> drivers;
		Dictionary d1;
		d1["source"] = StringName("stat.str");
		d1["destination"] = StringName("stat.atk");
		d1["ratio"] = 2.0f;
		drivers.push_back(d1);
		set->set_attribute_drivers(drivers);

		component->set_attribute_base_value_by_tag("stat.str", 10.0f); // Atk = 20

		// Var 1: Driver + Flat Add (20 + 5 = 25)
		set->add_modifier("stat.atk", 5.0f, ASAttributeSet::MODIFIER_ADD);
		CHECK_ATTR_EQ(component, "stat.atk", 25.0f);

		// Var 2: Driver Update with Modifier (20*2 + 5 = 45)
		component->set_attribute_base_value_by_tag("stat.str", 20.0f);
		CHECK_ATTR_EQ(component, "stat.atk", 45.0f);

		// Var 3: Modifier Removal (Back to driven value: 20*2 = 40)
		set->remove_modifier("stat.atk", 5.0f, ASAttributeSet::MODIFIER_ADD);
		CHECK_ATTR_EQ(component, "stat.atk", 40.0f);
	}

	memdelete(component);
}

#endif // TEST_AS_ATTRIBUTE_DRIVERS_H
