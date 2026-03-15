/**************************************************************************/
/*  test_as_component.h                                                   */
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
#include "src/resources/as_ability.h"
#include "src/resources/as_attribute.h"
#include "src/resources/as_attribute_set.h"
#include "src/resources/as_cue.h"
#include "src/resources/as_effect.h"
#include "src/resources/as_package.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/core/as_effect_spec.h"
#include "modules/ability_system/resources/as_ability.h"
#include "modules/ability_system/resources/as_attribute.h"
#include "modules/ability_system/resources/as_attribute_set.h"
#include "modules/ability_system/resources/as_cue.h"
#include "modules/ability_system/resources/as_effect.h"
#include "modules/ability_system/resources/as_package.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("ASComponent Core") {
	ASComponent *asc = memnew(ASComponent);

	SUBCASE("Attribute Set Management") {
		Ref<ASAttributeSet> attr_set = memnew(ASAttributeSet);
		Ref<ASAttribute> health_attr = memnew(ASAttribute);
		health_attr->set_attribute_name("Health");
		health_attr->set_base_value(100.0f);

		attr_set->add_attribute_definition(health_attr);

		asc->add_attribute_set(attr_set);

		// Verify attribute was registered in the component
		CHECK(asc->has_attribute_by_tag("Health") == true);
		CHECK(asc->get_attribute_value_by_tag("Health") == 100.0f);
	}

	SUBCASE("Effect Application (Instant)") {
		Ref<ASAttributeSet> attr_set = memnew(ASAttributeSet);
		Ref<ASAttribute> health_attr = memnew(ASAttribute);
		health_attr->set_attribute_name("Health");
		health_attr->set_base_value(100.0f);
		health_attr->set_max_value(200.0f); // Increased max_value for Health
		attr_set->add_attribute_definition(health_attr);
		asc->add_attribute_set(attr_set);

		Ref<ASEffect> damage_effect = memnew(ASEffect);
		damage_effect->set_duration_policy(ASEffect::POLICY_INSTANT);
		damage_effect->add_modifier("Health", ASEffect::OP_ADD, -20.0f);

		asc->apply_effect_by_resource(damage_effect);

		CHECK(asc->get_attribute_value_by_tag("Health") == 80.0f);
	}

	SUBCASE("Effect Application (Duration)") {
		Ref<ASAttributeSet> attr_set = memnew(ASAttributeSet);
		Ref<ASAttribute> health_attr = memnew(ASAttribute);
		health_attr->set_attribute_name("Health");
		health_attr->set_base_value(100.0f);
		health_attr->set_max_value(200.0f); // Increased max_value for Health
		attr_set->add_attribute_definition(health_attr);
		asc->add_attribute_set(attr_set);

		Ref<ASEffect> buff_effect = memnew(ASEffect);
		buff_effect->set_duration_policy(ASEffect::POLICY_DURATION);
		buff_effect->set_duration_magnitude(2.0f);
		buff_effect->add_modifier("Health", ASEffect::OP_ADD, 50.0f);

		asc->apply_effect_by_resource(buff_effect);

		// Initial application
		CHECK(asc->get_attribute_value_by_tag("Health") == 150.0f);
		CHECK(asc->has_active_effect_by_resource(buff_effect) == true);

		// Tick 1s (Halfway)
		asc->tick(1.0f);
		CHECK(asc->get_attribute_value_by_tag("Health") == 150.0f);
		CHECK(asc->has_active_effect_by_resource(buff_effect) == true);

		// Tick another 1.1s (Expired)
		asc->tick(1.1f);
		CHECK(asc->get_attribute_value_by_tag("Health") == 100.0f);
		CHECK(asc->has_active_effect_by_resource(buff_effect) == false);
	}

	SUBCASE("Effect Stacking") {
		Ref<ASAttributeSet> attr_set = memnew(ASAttributeSet);
		Ref<ASAttribute> health_attr = memnew(ASAttribute);
		health_attr->set_attribute_name("Health");
		health_attr->set_base_value(100.0f);
		health_attr->set_max_value(1000.0f);
		attr_set->add_attribute_definition(health_attr);
		asc->add_attribute_set(attr_set);

		Ref<ASEffect> stack_effect = memnew(ASEffect);
		stack_effect->set_duration_policy(ASEffect::POLICY_DURATION);
		stack_effect->set_duration_magnitude(5.0f);
		stack_effect->add_modifier("Health", ASEffect::OP_ADD, 10.0f);

		MESSAGE("Testing STACK_OVERRIDE...");
		stack_effect->set_stacking_policy(ASEffect::STACK_OVERRIDE);
		asc->apply_effect_by_resource(stack_effect);
		asc->tick(4.0f); // 1s left

		{
			TypedArray<ASEffectSpec> active = asc->get_active_effects_debug();
			CHECK(active.size() == 1);
			Ref<ASEffectSpec> spec = active[0];
			CHECK(doctest::Approx(spec->get_duration_remaining()) == 1.0f);

			asc->apply_effect_by_resource(stack_effect); // Should reset to 5s
			active = asc->get_active_effects_debug();
			spec = active[0];
			CHECK(doctest::Approx(spec->get_duration_remaining()) == 5.0f);
			CHECK(asc->get_attribute_value_by_tag("Health") == 110.0f); // Should NOT add more value
		}

		asc->remove_effect_by_resource(stack_effect);

		MESSAGE("Testing STACK_INTENSITY...");
		stack_effect->set_stacking_policy(ASEffect::STACK_INTENSITY);
		asc->apply_effect_by_resource(stack_effect); // Stack 1: 110
		CHECK(asc->get_attribute_value_by_tag("Health") == 110.0f);

		asc->apply_effect_by_resource(stack_effect); // Stack 2: 120
		CHECK(asc->get_attribute_value_by_tag("Health") == 120.0f);

		asc->apply_effect_by_resource(stack_effect); // Stack 3: 130
		CHECK(asc->get_attribute_value_by_tag("Health") == 130.0f);

		asc->remove_effect_by_resource(stack_effect);
		CHECK(asc->get_attribute_value_by_tag("Health") == 100.0f);

		MESSAGE("Testing STACK_DURATION...");
		stack_effect->set_stacking_policy(ASEffect::STACK_DURATION);
		asc->apply_effect_by_resource(stack_effect); // 5s
		asc->tick(1.0f); // 4s left

		{
			TypedArray<ASEffectSpec> active = asc->get_active_effects_debug();
			CHECK(active.size() == 1);
			Ref<ASEffectSpec> spec = active[0];
			CHECK(doctest::Approx(spec->get_duration_remaining()) == 4.0f);

			asc->apply_effect_by_resource(stack_effect); // 4s + 5s = 9s
			active = asc->get_active_effects_debug();
			spec = active[0];
			CHECK(doctest::Approx(spec->get_duration_remaining()) == 9.0f);
		}

		asc->remove_effect_by_resource(stack_effect);
	}

	SUBCASE("Tag Management") {
		asc->add_tag("State.Stunned");
		CHECK(asc->has_tag("State.Stunned") == true);
		CHECK(asc->has_tag("State") == true); // Hierarchical check

		// Test get_tags
		TypedArray<StringName> tags = asc->get_tags();
		CHECK(tags.size() == 1);
		CHECK(StringName(tags[0]) == StringName("State.Stunned"));

		asc->remove_tag("State.Stunned");
		CHECK(asc->has_tag("State.Stunned") == false);
		CHECK(asc->get_tags().size() == 0);
	}

	SUBCASE("Ability Management (Resource vs Tag Parity)") {
		Ref<ASAbility> ability = memnew(ASAbility);
		ability->set_ability_tag("Ability.Test");
		ability->set_ability_name("Test Ability");
		ability->set_ability_duration(5.0f);
		ability->set_duration_policy(ASAbility::POLICY_DURATION);

		// Catalog must have the ability for activation to work in GDExtension
		Ref<ASContainer> container = memnew(ASContainer);
		TypedArray<ASAbility> abilities;
		abilities.push_back(ability);
		container->set_abilities(abilities);
		asc->set_container(container);

		// Unlock by resource
		asc->unlock_ability_by_resource(ability);
		CHECK(asc->is_ability_unlocked("Ability.Test") == true);
		CHECK(asc->can_activate_ability_by_tag("Ability.Test") == true);
		CHECK(asc->can_activate_ability_by_resource(ability) == true);

		// Activate by resource
		CHECK(asc->try_activate_ability_by_resource(ability) == true);
		CHECK(asc->is_on_cooldown("Ability.Test") == false); // No cooldown set

		// Cancel by resource
		asc->cancel_ability_by_resource(ability);

		// Lock by resource
		asc->lock_ability_by_resource(ability);
		CHECK(asc->is_ability_unlocked("Ability.Test") == false);
		CHECK(asc->can_activate_ability_by_tag("Ability.Test") == false);
	}

	SUBCASE("Effect Active Check Parity") {
		Ref<ASEffect> effect = memnew(ASEffect);
		effect->set_effect_tag("Effect.Test");
		effect->set_duration_policy(ASEffect::POLICY_INFINITE);

		asc->apply_effect_by_resource(effect);

		CHECK(asc->has_active_effect_by_resource(effect) == true);
		CHECK(asc->has_active_effect_by_tag("Effect.Test") == true);

		asc->remove_effect_by_tag("Effect.Test");

		CHECK(asc->has_active_effect_by_resource(effect) == false);
		CHECK(asc->has_active_effect_by_tag("Effect.Test") == false);
	}

	SUBCASE("ASPackage Integration") {
		Ref<ASAttributeSet> attr_set = memnew(ASAttributeSet);
		Ref<ASAttribute> health_attr = memnew(ASAttribute);
		health_attr->set_attribute_name("Health");
		health_attr->set_base_value(100.0f);
		health_attr->set_max_value(1000.0f);
		attr_set->add_attribute_definition(health_attr);
		asc->add_attribute_set(attr_set);

		Ref<ASPackage> package = memnew(ASPackage);

		// 1. Instant Effect (Direct Resource)
		Ref<ASEffect> dmg = memnew(ASEffect);
		dmg->set_duration_policy(ASEffect::POLICY_INSTANT);
		dmg->add_modifier("Health", ASEffect::OP_ADD, -10.0f);
		package->add_effect(dmg);

		// 2. Effect Tag (Resolved via Container)
		Ref<ASContainer> container = memnew(ASContainer);
		Ref<ASEffect> tagged_dmg = memnew(ASEffect);
		tagged_dmg->set_effect_tag("effect.payload");
		tagged_dmg->set_duration_policy(ASEffect::POLICY_INSTANT);
		tagged_dmg->add_modifier("Health", ASEffect::OP_ADD, -5.0f);

		TypedArray<ASEffect> catalog;
		catalog.push_back(tagged_dmg);
		container->set_effects(catalog);
		asc->set_container(container);

		package->add_effect_tag("effect.payload");

		// Apply Package (100 - 10 - 5 = 85)
		asc->apply_package(package);
		CHECK(asc->get_attribute_value_by_tag("Health") == 85.0f);

		// Apply with Level 2.0 (10*2 + 5*2 = 30) -> 85 - 30 = 55
		asc->apply_package(package, 2.0f);
		CHECK(asc->get_attribute_value_by_tag("Health") == 55.0f);
	}

	// Clean up
	memdelete(asc);
}
