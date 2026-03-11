/**************************************************************************/
/*  test_helpers.h                                                        */
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

#include "doctest.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/core/ability_system_ability_spec.h"
#include "src/core/ability_system_effect_spec.h"
#include "src/resources/ability_system_ability.h"
#include "src/resources/ability_system_ability_container.h"
#include "src/resources/ability_system_attribute.h"
#include "src/resources/ability_system_attribute_set.h"
#include "src/resources/ability_system_effect.h"
#include "src/scene/ability_system_component.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#else
#include "core/object/class_db.h"
#include "modules/ability_system/core/ability_system_ability_spec.h"
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_ability_container.h"
#include "modules/ability_system/resources/ability_system_attribute.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/scene/ability_system_component.h"
#include "scene/main/node.h"
#endif

using namespace godot;

/**
 * Helper macros and functions for unit testing the Ability System.
 */

// --- Factory Helpers (Atomic) ---

inline Ref<AbilitySystemAttribute> create_test_attribute(const StringName &p_name, float p_base = 100.0f, float p_min = 0.0f, float p_max = 999.0f) {
	Ref<AbilitySystemAttribute> attr;
	attr.instantiate();
	attr->set_attribute_name(p_name);
	attr->set_base_value(p_base);
	attr->set_min_value(p_min);
	attr->set_max_value(p_max);
	return attr;
}

inline Ref<AbilitySystemEffect> create_test_effect(const StringName &p_name, AbilitySystemEffect::DurationPolicy p_policy = AbilitySystemEffect::POLICY_INSTANT, float p_duration = 0.0f) {
	Ref<AbilitySystemEffect> effect;
	effect.instantiate();
	effect->set_effect_tag(p_name);
	effect->set_duration_policy(p_policy);
	effect->set_duration_magnitude(p_duration);
	return effect;
}

inline Ref<AbilitySystemEffect> make_instant_effect(const StringName &p_name, const StringName &p_attr, float p_amount) {
	Ref<AbilitySystemEffect> effect = create_test_effect(p_name, AbilitySystemEffect::POLICY_INSTANT);
	effect->add_modifier(p_attr, AbilitySystemEffect::OP_ADD, p_amount);
	return effect;
}

inline Ref<AbilitySystemEffect> make_duration_effect(const StringName &p_name, float p_duration, const StringName &p_attr = "", float p_amount = 0.0f, AbilitySystemEffect::ModifierOp p_op = AbilitySystemEffect::OP_ADD, const StringName &p_granted_tag = "") {
	Ref<AbilitySystemEffect> effect = create_test_effect(p_name, AbilitySystemEffect::POLICY_DURATION, p_duration);
	if (p_attr != StringName()) {
		effect->add_modifier(p_attr, p_op, p_amount);
	}
	if (p_granted_tag != StringName()) {
		TypedArray<StringName> tags;
		tags.push_back(p_granted_tag);
		effect->set_granted_tags(tags);
	}
	return effect;
}

inline Ref<AbilitySystemAbility> create_test_ability(const StringName &p_name, const StringName &p_tag) {
	Ref<AbilitySystemAbility> ability;
	ability.instantiate();
	ability->set_ability_name(p_name);
	ability->set_ability_tag(p_tag);
	return ability;
}

inline Ref<AbilitySystemAbility> make_ability(const StringName &p_name, const StringName &p_tag, Ref<AbilitySystemEffect> p_effect = nullptr, float p_cost_amount = 0.0f, const StringName &p_cost_attr = "") {
	Ref<AbilitySystemAbility> ability = create_test_ability(p_name, p_tag);
	if (p_effect.is_valid()) {
		ability->set_effect(p_effect);
	}
	if (p_cost_attr != StringName()) {
		ability->add_cost(p_cost_attr, p_cost_amount);
	}
	return ability;
}

// --- Scenario Factories (Complex) ---

inline AbilitySystemComponent *make_standard_asc(float p_health = 100.0f, float p_mana = 80.0f, float p_stamina = 80.0f) {
	Ref<AbilitySystemAttributeSet> set;
	set.instantiate();
	set->add_attribute_definition(create_test_attribute("Health", p_health));
	set->add_attribute_definition(create_test_attribute("Mana", p_mana));
	set->add_attribute_definition(create_test_attribute("Stamina", p_stamina));

	AbilitySystemComponent *asc = memnew(AbilitySystemComponent);
	asc->add_attribute_set(set);
	return asc;
}

inline AbilitySystemComponent *make_warrior_asc() {
	return make_standard_asc(150.0f, 20.0f, 120.0f); // High health/stamina, low mana
}

inline AbilitySystemComponent *make_mage_asc() {
	return make_standard_asc(80.0f, 200.0f, 50.0f); // Low health/stamina, high mana
}

// --- Validation Helpers ---

#define CHECK_ATTR_EQ(asc, name, value) \
	CHECK(Math::is_equal_approx(asc->get_attribute_value_by_tag(name), (float)(value)))

#define CHECK_HAS_TAG(asc, tag) \
	CHECK(asc->has_tag(tag) == true)

#define CHECK_NO_TAG(asc, tag) \
	CHECK(asc->has_tag(tag) == false)

#define CHECK_HAS_EFFECT(asc, effect) \
	CHECK(asc->has_active_effect_by_resource(effect) == true)

#define CHECK_NO_EFFECT(asc, effect) \
	CHECK(asc->has_active_effect_by_resource(effect) == false)

#define CHECK_COOLDOWN(asc, tag, expected) \
	CHECK(asc->is_on_cooldown(tag) == expected)

// --- Debug Helpers ---

inline String debug_active_effects(AbilitySystemComponent *p_asc) {
	String out = "[";
	TypedArray<AbilitySystemEffectSpec> effects = p_asc->get_active_effects_debug();
	for (int i = 0; i < effects.size(); i++) {
		Ref<AbilitySystemEffectSpec> spec = effects[i];
		if (spec.is_valid() && spec->get_effect().is_valid()) {
			out += spec->get_effect()->get_effect_tag();
			if (i < effects.size() - 1) {
				out += ", ";
			}
		}
	}
	out += "]";
	return out;
}

inline String debug_unlocked_abilities(AbilitySystemComponent *p_asc) {
	String out = "[Unlocked: ";
	TypedArray<AbilitySystemAbilitySpec> unlocked = p_asc->get_unlocked_abilities_debug();
	for (int i = 0; i < unlocked.size(); i++) {
		Ref<AbilitySystemAbilitySpec> spec = unlocked[i];
		if (spec.is_valid() && spec->get_ability().is_valid()) {
			out += spec->get_ability()->get_ability_name();
			if (i < unlocked.size() - 1) {
				out += ", ";
			}
		}
	}
	out += " | Active: ";
	TypedArray<AbilitySystemAbilitySpec> active = p_asc->get_active_abilities_debug();
	for (int i = 0; i < active.size(); i++) {
		Ref<AbilitySystemAbilitySpec> spec = active[i];
		if (spec.is_valid() && spec->get_ability().is_valid()) {
			out += spec->get_ability()->get_ability_name();
			if (i < active.size() - 1) {
				out += ", ";
			}
		}
	}
	out += "]";
	return out;
}

// Specialized check macros that print debug info on failure
#define CHECK_ATTR_MSG(asc, name, value)                                               \
	{                                                                                  \
		float cur = asc->get_attribute_value_by_tag(name);                             \
		if (!Math::is_equal_approx(cur, (float)(value))) {                             \
			INFO("Attribute '", name, "' mismatch. Expected: ", value, " Got: ", cur); \
			INFO(debug_active_effects(asc));                                           \
			CHECK(false);                                                              \
		}                                                                              \
	}

#define CHECK_TAG_MSG(asc, tag, expected)                                          \
	{                                                                              \
		bool has = asc->has_tag(tag);                                              \
		if (has != expected) {                                                     \
			INFO("Tag '", tag, "' mismatch. Expected: ", expected, " Got: ", has); \
			INFO("Current Tags: ", asc->get_tags());                               \
			INFO(debug_active_effects(asc));                                       \
			CHECK(false);                                                          \
		}                                                                          \
	}

#define CHECK_ABILITY_ACTIVE_MSG(asc, tag, expected)                                             \
	{                                                                                            \
		bool active = asc->is_ability_active(tag);                                               \
		if (active != expected) {                                                                \
			INFO("Ability Active '", tag, "' mismatch. Expected: ", expected, " Got: ", active); \
			INFO(debug_unlocked_abilities(asc));                                                 \
			INFO(debug_active_effects(asc));                                                     \
			CHECK(false);                                                                        \
		}                                                                                        \
	}
