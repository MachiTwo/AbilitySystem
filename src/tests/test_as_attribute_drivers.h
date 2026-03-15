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

#include "doctest.h"
#include "src/resources/as_attribute.h"
#include "src/resources/as_attribute_set.h"
#include "src/scene/as_component.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AbilitySystem] Attribute Drivers (300% Coverage)") {
	ASComponent *component = memnew(ASComponent);
	Ref<ASAttributeSet> set;
	set.instantiate();

	// Setup 3 Base Attributes
	Ref<ASAttribute> attr_str;
	attr_str.instantiate();
	attr_str->set_attribute_name("stat.str");
	Ref<ASAttribute> attr_dex;
	attr_dex.instantiate();
	attr_dex->set_attribute_name("stat.dex");
	Ref<ASAttribute> attr_int;
	attr_int.instantiate();
	attr_int->set_attribute_name("stat.int");

	// Setup 3 Derived Attributes
	Ref<ASAttribute> attr_atk;
	attr_atk.instantiate();
	attr_atk->set_attribute_name("stat.atk");
	Ref<ASAttribute> attr_spd;
	attr_spd.instantiate();
	attr_spd->set_attribute_name("stat.spd");
	Ref<ASAttribute> attr_man;
	attr_man.instantiate();
	attr_man->set_attribute_name("stat.mana");

	set->add_attribute_definition(attr_str);
	set->add_attribute_definition(attr_dex);
	set->add_attribute_definition(attr_int);
	set->add_attribute_definition(attr_atk);
	set->add_attribute_definition(attr_spd);
	set->add_attribute_definition(attr_man);

	component->add_attribute_set(set);

	SUBCASE("Triple Attribute Relationship") {
		TypedArray<Dictionary> drivers;

		Dictionary d1;
		d1["source"] = StringName("stat.str");
		d1["destination"] = StringName("stat.atk");
		d1["ratio"] = 2.0f;
		Dictionary d2;
		d2["source"] = StringName("stat.dex");
		d2["destination"] = StringName("stat.spd");
		d2["ratio"] = 1.5f;
		Dictionary d3;
		d3["source"] = StringName("stat.int");
		d3["destination"] = StringName("stat.mana");
		d3["ratio"] = 10.0f;

		drivers.push_back(d1);
		drivers.push_back(d2);
		drivers.push_back(d3);

		set->set_attribute_drivers(drivers);

		// Test Variation 1: Strength -> Attack
		component->set_attribute_base_value_by_tag("stat.str", 10.0f);
		CHECK(component->get_attribute_value_by_tag("stat.atk") == 20.0f);

		// Test Variation 2: Dexterity -> Speed
		component->set_attribute_base_value_by_tag("stat.dex", 10.0f);
		CHECK(component->get_attribute_value_by_tag("stat.spd") == 15.0f);

		// Test Variation 3: Intelligence -> Mana
		component->set_attribute_base_value_by_tag("stat.int", 5.0f);
		CHECK(component->get_attribute_value_by_tag("stat.mana") == 50.0f);
	}

	SUBCASE("Multiple Sources for Single Destination") {
		TypedArray<Dictionary> drivers;
		// ATK is driven by STR (x2) and DEX (x1)
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

		component->set_attribute_base_value_by_tag("stat.str", 10.0f);
		component->set_attribute_base_value_by_tag("stat.dex", 5.0f);

		// Result: 10*2 + 5*1 = 25
		CHECK(component->get_attribute_value_by_tag("stat.atk") == 25.0f);
	}

	SUBCASE("Driver + Modifier Interaction") {
		TypedArray<Dictionary> drivers;
		Dictionary d1;
		d1["source"] = StringName("stat.str");
		d1["destination"] = StringName("stat.atk");
		d1["ratio"] = 2.0f;
		drivers.push_back(d1);
		set->set_attribute_drivers(drivers);

		// 1. Base Driver
		component->set_attribute_base_value_by_tag("stat.str", 10.0f); // Atk = 20

		// 2. Add Modifier directly to ATK
		set->add_modifier("stat.atk", 5.0f); // 20 + 5 = 25
		CHECK(component->get_attribute_value_by_tag("stat.atk") == 25.0f);

		// 3. Change STR (Driver should update)
		component->set_attribute_base_value_by_tag("stat.str", 20.0f); // 20*2 + 5 = 45
		CHECK(component->get_attribute_value_by_tag("stat.atk") == 45.0f);
	}

	memdelete(component);
}

#endif // TEST_AS_ATTRIBUTE_DRIVERS_H
