/**************************************************************************/
/*  test_as_delivery.h                                                    */
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
#include "src/core/as_effect_spec.h"
#include "src/resources/as_attribute_set.h"
#include "src/resources/as_cue.h"
#include "src/resources/as_effect.h"
#include "src/resources/as_package.h"
#include "src/scene/as_component.h"
#include "src/scene/as_delivery.h"
#include "src/tests/doctest.h"
#include <godot_cpp/classes/node.hpp>
#else
#include "modules/ability_system/core/as_effect_spec.h"
#include "modules/ability_system/resources/as_attribute_set.h"
#include "modules/ability_system/resources/as_cue.h"
#include "modules/ability_system/resources/as_effect.h"
#include "modules/ability_system/resources/as_package.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/scene/as_delivery.h"
#include "modules/ability_system/tests/doctest.h"
#include "scene/main/node.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AS] ASDelivery V2 Testing") {
	ASComponent *source_asc = memnew(ASComponent);
	ASComponent *target_asc = memnew(ASComponent);
	ASDelivery *delivery = memnew(ASDelivery);

	// Setup AttributeSet for target
	Ref<ASAttributeSet> attr_set;
	attr_set.instantiate();
	Ref<ASAttribute> attr_health;
	attr_health.instantiate();
	attr_health->set_attribute_name("Health");
	attr_set->add_attribute_definition(attr_health);
	attr_set->set_attribute_base_value("Health", 100.0f);
	target_asc->add_attribute_set(attr_set);

	// Setup ASPackage
	Ref<ASPackage> package;
	package.instantiate();
	Ref<ASEffect> damage_effect;
	damage_effect.instantiate();
	damage_effect->set_effect_tag("test.damage");
	damage_effect->set_duration_policy(ASEffect::POLICY_INSTANT);
	damage_effect->add_modifier("Health", ASEffect::OP_ADD, -10.0f);
	package->add_effect(damage_effect);

	delivery->set_package(package);
	delivery->set_source_component(source_asc);

	SUBCASE("Lifecycle: Activation and Timer") {
		delivery->set_life_span(1.0f);
		CHECK(delivery->get_active() == false);
		CHECK(delivery->is_delivery_valid() == false);

		delivery->activate();
		CHECK(delivery->get_active() == true);
		CHECK(delivery->is_delivery_valid() == true);

		delivery->deactivate();
		CHECK(delivery->get_active() == false);
	}

	SUBCASE("Filtering: Target Groups") {
		Node *target_node = memnew(Node);
		target_node->add_child(target_asc);

		TypedArray<StringName> groups;
		groups.push_back("enemies");
		delivery->set_target_groups(groups);
		delivery->activate();

		// Should fail delivery because target_node is NOT in "enemies" group
		delivery->deliver(target_node);
		CHECK(target_asc->get_attribute_value_by_tag("Health") == 100.0f);

		// Now add it to group
		target_node->add_to_group("enemies");
		delivery->deliver(target_node);
		CHECK(target_asc->get_attribute_value_by_tag("Health") == 90.0f);

		target_node->remove_child(target_asc);
		memdelete(target_node);
	}

	SUBCASE("Lifecycle: One Shot") {
		Node *target_node = memnew(Node);
		target_node->add_child(target_asc);
		target_asc->set_attribute_base_value_by_tag("Health", 100.0f);

		delivery->set_one_shot(true);
		delivery->set_target_groups(TypedArray<StringName>()); // Clear groups
		delivery->activate();

		delivery->deliver(target_node);
		CHECK(target_asc->get_attribute_value_by_tag("Health") == 90.0f);
		CHECK(delivery->get_active() == false); // Should be deactivated after one shot

		// Second delivery should fail because inactive
		delivery->deliver(target_node);
		CHECK(target_asc->get_attribute_value_by_tag("Health") == 90.0f);

		target_node->remove_child(target_asc);
		memdelete(target_node);
	}

	memdelete(delivery);
	memdelete(target_asc);
	memdelete(source_asc);
}
