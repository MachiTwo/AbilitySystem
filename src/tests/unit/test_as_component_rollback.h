/**************************************************************************/
/*  test_as_component_rollback.h                                          */
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

#ifndef TEST_AS_COMPONENT_ROLLBACK_H
#define TEST_AS_COMPONENT_ROLLBACK_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/resources/as_attribute.h"
#include "src/resources/as_attribute_set.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/resources/as_attribute.h"
#include "modules/ability_system/resources/as_attribute_set.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

// [RED STATE] — Testing newly implemented Rollback API
TEST_CASE("ASComponent - Rollback Infrastructure") {
	ASComponent *asc = memnew(ASComponent);
	Ref<ASAttributeSet> attr_set = memnew(ASAttributeSet);
	Ref<ASAttribute> health = memnew(ASAttribute);
	health->set_attribute_name("Health");
	attr_set->add_attribute_definition(health);
	asc->add_attribute_set(attr_set);

	SUBCASE("Attribute Rollback - 3 Variations") {
		// Var 1: Base scenario - Simple modification and return to original state
		asc->current_tick = 10;
		attr_set->set_attribute_base_value("Health", 100.0);
		asc->capture_snapshot(); // Needs to save at current_tick

		asc->current_tick = 11;
		attr_set->set_attribute_base_value("Health", 80.0);

		asc->rollback_to_tick(10);
		CHECK(attr_set->get_attribute_base_value("Health") == 100.0);
		CHECK(asc->current_tick == 10);

		// Var 2: Edge scenario - Rollback to non-existent tick in cache
		asc->rollback_to_tick(1);
		// Should not crash, just maintain current state if tick is not found
		CHECK(attr_set->get_attribute_base_value("Health") == 100.0);

		// Var 3: Composite scenario - Multiple changes followed by deep rollback
		asc->current_tick = 10;
		attr_set->set_attribute_base_value("Health", 100.0);
		asc->capture_snapshot();
		asc->current_tick = 11;
		attr_set->set_attribute_base_value("Health", 90.0);
		asc->capture_snapshot();
		asc->current_tick = 12;
		attr_set->set_attribute_base_value("Health", 80.0);
		asc->capture_snapshot();

		asc->rollback_to_tick(10);
		CHECK(attr_set->get_attribute_base_value("Health") == 100.0);
	}

	SUBCASE("Tag Historical Clean - 3 Variations") {
		StringName tag_stun = "State.Stun";

		// Var 1: Happy path - Rollback removes tags added in the "future"
		asc->current_tick = 20;
		asc->capture_snapshot();

		asc->current_tick = 21;
		asc->add_tag(tag_stun);
		CHECK(asc->has_tag(tag_stun));

		asc->rollback_to_tick(20);
		CHECK(asc->has_tag(tag_stun) == false);

		// Var 2: Negative scenario - Rollback to the same tick (IDEMPOTENCY)
		asc->current_tick = 30;
		asc->add_tag(tag_stun);
		asc->capture_snapshot();
		asc->rollback_to_tick(30);
		CHECK(asc->has_tag(tag_stun) == true);

		// Var 3: Composite scenario - Tag and event hierarchy
		asc->current_tick = 40;
		asc->capture_snapshot();
		asc->current_tick = 41;
		// Testing event occurrence if supported
		// asc->dispatch_event("Event.Damage", nullptr, 10.0);
		// CHECK(asc->has_event_occurred("Event.Damage", 1.0));

		asc->rollback_to_tick(40);
		// CHECK(asc->has_event_occurred("Event.Damage", 1.0) == false);
	}

	memdelete(asc);
}

#endif // TEST_AS_COMPONENT_ROLLBACK_H
