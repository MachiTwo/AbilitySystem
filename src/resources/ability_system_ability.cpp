/**************************************************************************/
/*  ability_system_ability.cpp                                            */
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

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/resources/ability_system_ability.h"
#include "src/core/ability_system.h"
#include "src/core/ability_system_ability_spec.h"
#include "src/core/ability_system_cue_spec.h"
#include "src/core/ability_system_effect_spec.h"
#include "src/core/ability_system_tag_spec.h"
#include "src/resources/ability_system_cue.h"
#include "src/resources/ability_system_effect.h"
#include "src/scene/ability_system_component.h"
#else
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/core/ability_system_ability_spec.h"
#include "modules/ability_system/core/ability_system_cue_spec.h"
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/core/ability_system_tag_spec.h"
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/scene/ability_system_component.h"
#endif

namespace godot {

void AbilitySystemAbility::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_ability_name", "name"), &AbilitySystemAbility::set_ability_name);
	ClassDB::bind_method(D_METHOD("get_ability_name"), &AbilitySystemAbility::get_ability_name);
	ClassDB::bind_method(D_METHOD("set_ability_tag", "tag"), &AbilitySystemAbility::set_ability_tag);
	ClassDB::bind_method(D_METHOD("get_ability_tag"), &AbilitySystemAbility::get_ability_tag);

	ClassDB::bind_method(D_METHOD("set_activation_owned_tags", "tags"), &AbilitySystemAbility::set_activation_owned_tags);
	ClassDB::bind_method(D_METHOD("get_activation_owned_tags"), &AbilitySystemAbility::get_activation_owned_tags);

	ClassDB::bind_method(D_METHOD("set_activation_required_tags", "tags"), &AbilitySystemAbility::set_activation_required_tags);
	ClassDB::bind_method(D_METHOD("get_activation_required_tags"), &AbilitySystemAbility::get_activation_required_tags);

	ClassDB::bind_method(D_METHOD("set_activation_blocked_tags", "tags"), &AbilitySystemAbility::set_activation_blocked_tags);
	ClassDB::bind_method(D_METHOD("get_activation_blocked_tags"), &AbilitySystemAbility::get_activation_blocked_tags);

	ClassDB::bind_method(D_METHOD("set_cues", "cues"), &AbilitySystemAbility::set_cues);
	ClassDB::bind_method(D_METHOD("get_cues"), &AbilitySystemAbility::get_cues);

	ClassDB::bind_method(D_METHOD("set_effect", "effect"), &AbilitySystemAbility::set_effect);
	ClassDB::bind_method(D_METHOD("get_effect"), &AbilitySystemAbility::get_effect);

	// Duration methods
	ClassDB::bind_method(D_METHOD("set_duration_policy", "policy"), &AbilitySystemAbility::set_duration_policy);
	ClassDB::bind_method(D_METHOD("get_duration_policy"), &AbilitySystemAbility::get_duration_policy);
	ClassDB::bind_method(D_METHOD("set_ability_duration", "duration"), &AbilitySystemAbility::set_ability_duration);
	ClassDB::bind_method(D_METHOD("get_ability_duration"), &AbilitySystemAbility::get_ability_duration);
	ClassDB::bind_method(D_METHOD("set_use_custom_duration", "use"), &AbilitySystemAbility::set_use_custom_duration);
	ClassDB::bind_method(D_METHOD("get_use_custom_duration"), &AbilitySystemAbility::get_use_custom_duration);
	ClassDB::bind_method(D_METHOD("calculate_ability_duration", "owner"), &AbilitySystemAbility::calculate_ability_duration);

	// Cooldown methods
	ClassDB::bind_method(D_METHOD("set_cooldown_duration", "duration"), &AbilitySystemAbility::set_cooldown_duration);
	ClassDB::bind_method(D_METHOD("get_cooldown_duration"), &AbilitySystemAbility::get_cooldown_duration);
	ClassDB::bind_method(D_METHOD("set_cooldown_tags", "tags"), &AbilitySystemAbility::set_cooldown_tags);
	ClassDB::bind_method(D_METHOD("get_cooldown_tags"), &AbilitySystemAbility::get_cooldown_tags);
	ClassDB::bind_method(D_METHOD("set_use_custom_cooldown", "use"), &AbilitySystemAbility::set_use_custom_cooldown);
	ClassDB::bind_method(D_METHOD("get_use_custom_cooldown"), &AbilitySystemAbility::get_use_custom_cooldown);

	// Cost methods
	ClassDB::bind_method(D_METHOD("set_costs", "costs"), &AbilitySystemAbility::set_costs);
	ClassDB::bind_method(D_METHOD("get_costs"), &AbilitySystemAbility::get_costs);
	ClassDB::bind_method(D_METHOD("set_use_custom_costs", "use"), &AbilitySystemAbility::set_use_custom_costs);
	ClassDB::bind_method(D_METHOD("get_use_custom_costs"), &AbilitySystemAbility::get_use_custom_costs);
	ClassDB::bind_method(D_METHOD("add_cost", "attribute", "amount"), &AbilitySystemAbility::add_cost);
	ClassDB::bind_method(D_METHOD("remove_cost", "attribute"), &AbilitySystemAbility::remove_cost);
	ClassDB::bind_method(D_METHOD("get_cost_amount", "attribute"), &AbilitySystemAbility::get_cost_amount);
	ClassDB::bind_method(D_METHOD("can_afford_costs", "owner", "spec"), &AbilitySystemAbility::can_afford_costs, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("apply_costs", "owner", "spec"), &AbilitySystemAbility::apply_costs, DEFVAL(Variant()));

	// Requirement methods
	ClassDB::bind_method(D_METHOD("set_requirements", "requirements"), &AbilitySystemAbility::set_requirements);
	ClassDB::bind_method(D_METHOD("get_requirements"), &AbilitySystemAbility::get_requirements);
	ClassDB::bind_method(D_METHOD("get_requirement_count"), &AbilitySystemAbility::get_requirement_count);
	ClassDB::bind_method(D_METHOD("add_requirement", "attribute", "amount"), &AbilitySystemAbility::add_requirement);
	ClassDB::bind_method(D_METHOD("remove_requirement", "attribute"), &AbilitySystemAbility::remove_requirement);
	ClassDB::bind_method(D_METHOD("get_requirement_amount", "attribute"), &AbilitySystemAbility::get_requirement_amount);
	ClassDB::bind_method(D_METHOD("can_satisfy_requirements", "owner", "spec"), &AbilitySystemAbility::can_satisfy_requirements, DEFVAL(Variant()));

	ClassDB::bind_method(D_METHOD("can_activate_ability", "owner", "spec"), &AbilitySystemAbility::can_activate_ability, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("activate_ability", "owner", "spec", "target_node"), &AbilitySystemAbility::activate_ability, DEFVAL(Variant()), DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("end_ability", "owner", "spec"), &AbilitySystemAbility::end_ability, DEFVAL(Variant()));

	GDVIRTUAL_BIND(_on_activate_ability, "owner", "spec");
	GDVIRTUAL_BIND(_on_can_activate_ability, "owner", "spec");
	GDVIRTUAL_BIND(_on_end_ability, "owner", "spec");

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "ability_name"), "set_ability_name", "get_ability_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "ability_tag"), "set_ability_tag", "get_ability_tag");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "activation_owned_tags", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_activation_owned_tags", "get_activation_owned_tags");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "activation_required_tags", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_activation_required_tags", "get_activation_required_tags");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "activation_blocked_tags", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_activation_blocked_tags", "get_activation_blocked_tags");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "cues", PROPERTY_HINT_ARRAY_TYPE, "AbilitySystemCue"), "set_cues", "get_cues");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "effect", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemEffect"), "set_effect", "get_effect");

	ADD_PROPERTY(PropertyInfo(Variant::INT, "ability_duration_policy", PROPERTY_HINT_ENUM, "Instant,Duration,Infinite"), "set_duration_policy", "get_duration_policy");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ability_duration"), "set_ability_duration", "get_ability_duration");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ability_use_custom_duration", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_use_custom_duration", "get_use_custom_duration");

	BIND_ENUM_CONSTANT(POLICY_INSTANT);
	BIND_ENUM_CONSTANT(POLICY_DURATION);
	BIND_ENUM_CONSTANT(POLICY_INFINITE);

	ADD_GROUP("Cooldown", "cooldown_");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cooldown_duration"), "set_cooldown_duration", "get_cooldown_duration");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "cooldown_tags", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_cooldown_tags", "get_cooldown_tags");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cooldown_use_custom", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_use_custom_cooldown", "get_use_custom_cooldown");

	ADD_GROUP("Costs", "costs_");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "costs", PROPERTY_HINT_ARRAY_TYPE, "Dictionary"), "set_costs", "get_costs");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "costs_use_custom", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_use_custom_costs", "get_use_custom_costs");

	ADD_GROUP("Requirements", "requirements_");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "requirements", PROPERTY_HINT_ARRAY_TYPE, "Dictionary"), "set_requirements", "get_requirements");
}

bool AbilitySystemAbility::can_activate_ability(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec) const {
	ERR_FAIL_NULL_V(p_owner, false);

	if (!p_owner->get_owned_tags()->has_all_tags(activation_required_tags)) {
		return false;
	}

	if (p_owner->get_owned_tags()->has_any_tags(activation_blocked_tags)) {
		return false;
	}

	if (ability_tag != StringName() && p_owner->is_on_cooldown(ability_tag)) {
		return false;
	}

	if (!can_satisfy_requirements(p_owner, p_spec)) {
		return false;
	}

	if (!can_afford_costs(p_owner, p_spec)) {
		return false;
	}

	bool script_ret = true;
	if (GDVIRTUAL_CALL(_on_can_activate_ability, p_owner, p_spec, script_ret)) {
		if (!script_ret) {
			return false;
		}
	}

	return true;
}

void AbilitySystemAbility::activate_ability(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec, Object *p_target_node) {
	ERR_FAIL_COND(p_spec.is_null());
	p_spec->activate(p_target_node);
}

void AbilitySystemAbility::end_ability(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec) {
	ERR_FAIL_COND(p_spec.is_null());
	p_spec->deactivate();
}

void AbilitySystemAbility::set_effect(Ref<AbilitySystemEffect> p_effect) {
	effect = p_effect;
}

Ref<AbilitySystemEffect> AbilitySystemAbility::get_effect() const {
	return effect;
}

void AbilitySystemAbility::set_duration_policy(DurationPolicy p_policy) {
	duration_policy = p_policy;
}

AbilitySystemAbility::DurationPolicy AbilitySystemAbility::get_duration_policy() const {
	return duration_policy;
}

void AbilitySystemAbility::set_ability_duration(float p_duration) {
	ability_duration = p_duration;
}

float AbilitySystemAbility::get_ability_duration() const {
	return ability_duration;
}

void AbilitySystemAbility::set_use_custom_duration(bool p_use) {
	use_custom_duration = p_use;
}

bool AbilitySystemAbility::get_use_custom_duration() const {
	return use_custom_duration;
}

float AbilitySystemAbility::calculate_ability_duration(AbilitySystemComponent *p_owner) const {
	float duration = ability_duration;
	if (use_custom_duration && p_owner) {
		p_owner->GDVIRTUAL_CALL(_on_calculate_custom_magnitude, Variant(), -1, duration);
	}
	return duration;
}

String AbilitySystemAbility::get_ability_name() const {
	return ability_name;
}
StringName AbilitySystemAbility::get_ability_tag() const {
	return ability_tag;
}
TypedArray<StringName> AbilitySystemAbility::get_activation_owned_tags() const {
	return activation_owned_tags;
}
TypedArray<StringName> AbilitySystemAbility::get_activation_required_tags() const {
	return activation_required_tags;
}
TypedArray<StringName> AbilitySystemAbility::get_activation_blocked_tags() const {
	return activation_blocked_tags;
}
TypedArray<AbilitySystemCue> AbilitySystemAbility::get_cues() const {
	return cues;
}

void AbilitySystemAbility::set_cooldown_duration(float p_duration) {
	cooldown_duration = p_duration;
}
float AbilitySystemAbility::get_cooldown_duration() const {
	return cooldown_duration;
}
void AbilitySystemAbility::set_cooldown_tags(const TypedArray<StringName> &p_tags) {
	cooldown_tags = p_tags;
	if (AbilitySystem::get_singleton()) {
		for (int i = 0; i < p_tags.size(); i++) {
			AbilitySystem::get_singleton()->register_tag(p_tags[i], AbilitySystem::TAG_TYPE_CONDITIONAL);
		}
	}
}
TypedArray<StringName> AbilitySystemAbility::get_cooldown_tags() const {
	return cooldown_tags;
}
bool AbilitySystemAbility::get_use_custom_cooldown() const {
	return use_custom_cooldown;
}
void AbilitySystemAbility::set_use_custom_cooldown(bool p_use) {
	use_custom_cooldown = p_use;
}

TypedArray<Dictionary> AbilitySystemAbility::get_costs() const {
	return costs;
}
void AbilitySystemAbility::set_costs(const TypedArray<Dictionary> &p_costs) {
	costs = p_costs;
}
bool AbilitySystemAbility::get_use_custom_costs() const {
	return use_custom_costs;
}
void AbilitySystemAbility::set_use_custom_costs(bool p_use) {
	use_custom_costs = p_use;
}

void AbilitySystemAbility::set_requirements(const TypedArray<Dictionary> &p_requirements) {
	requirements = p_requirements;
}

TypedArray<Dictionary> AbilitySystemAbility::get_requirements() const {
	return requirements;
}

void AbilitySystemAbility::add_requirement(const StringName &p_attribute, float p_amount) {
	Dictionary req;
	req["attribute"] = p_attribute;
	req["amount"] = p_amount;
	requirements.append(req);
}

bool AbilitySystemAbility::remove_requirement(const StringName &p_attribute) {
	for (int i = 0; i < requirements.size(); i++) {
		Dictionary req = requirements[i];
		StringName attr = req["attribute"];
		if (req.has("attribute") && attr == p_attribute) {
			requirements.remove_at(i);
			return true;
		}
	}
	return false;
}

float AbilitySystemAbility::get_requirement_amount(const StringName &p_attribute) const {
	for (int i = 0; i < requirements.size(); i++) {
		Dictionary req = requirements[i];
		StringName attr = req["attribute"];
		if (req.has("attribute") && attr == p_attribute) {
			return req["amount"];
		}
	}
	return 0.0f;
}

bool AbilitySystemAbility::can_satisfy_requirements(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec) const {
	for (int i = 0; i < requirements.size(); i++) {
		Dictionary req = requirements[i];
		if (req.has("attribute") && req.has("amount")) {
			StringName attribute = req["attribute"];
			float amount = req["amount"];

			float current_value = p_owner->get_attribute_value_by_tag(attribute);
			if (current_value < amount) {
				return false;
			}
		}
	}
	return true;
}

void AbilitySystemAbility::set_activation_owned_tags(const TypedArray<StringName> &p_tags) {
	activation_owned_tags = p_tags;
	if (AbilitySystem::get_singleton()) {
		for (int i = 0; i < p_tags.size(); i++) {
			AbilitySystem::get_singleton()->register_tag(p_tags[i], AbilitySystem::TAG_TYPE_CONDITIONAL);
		}
	}
}

void AbilitySystemAbility::set_activation_required_tags(const TypedArray<StringName> &p_tags) {
	activation_required_tags = p_tags;
	if (AbilitySystem::get_singleton()) {
		for (int i = 0; i < p_tags.size(); i++) {
			AbilitySystem::get_singleton()->register_tag(p_tags[i], AbilitySystem::TAG_TYPE_CONDITIONAL);
		}
	}
}

void AbilitySystemAbility::set_activation_blocked_tags(const TypedArray<StringName> &p_tags) {
	activation_blocked_tags = p_tags;
	if (AbilitySystem::get_singleton()) {
		for (int i = 0; i < p_tags.size(); i++) {
			AbilitySystem::get_singleton()->register_tag(p_tags[i], AbilitySystem::TAG_TYPE_CONDITIONAL);
		}
	}
}

void AbilitySystemAbility::set_cues(const TypedArray<AbilitySystemCue> &p_cues) {
	cues = p_cues;
	if (ability_tag != StringName()) {
		for (int i = 0; i < cues.size(); i++) {
			Ref<AbilitySystemCue> cue = cues[i];
			if (cue.is_valid()) {
				cue->set_cue_tag(ability_tag);
			}
		}
	}
}

void AbilitySystemAbility::set_ability_name(const String &p_name) {
	if (ability_name == p_name) {
		return;
	}

	AbilitySystem *as = AbilitySystem::get_singleton();
	if (as) {
		if (!ability_name.is_empty()) {
			as->unregister_resource_name(ability_name);
		}
		as->register_resource_name(p_name, get_instance_id());
	}
	ability_name = p_name;
}

void AbilitySystemAbility::set_ability_tag(const StringName &p_tag) {
	ability_tag = p_tag;
	if (AbilitySystem::get_singleton()) {
		AbilitySystem::get_singleton()->register_tag(p_tag, AbilitySystem::TAG_TYPE_NAME, get_instance_id());
	}

	for (int i = 0; i < cues.size(); i++) {
		Ref<AbilitySystemCue> cue = cues[i];
		if (cue.is_valid()) {
			cue->set_cue_tag(p_tag);
		}
	}
}

void AbilitySystemAbility::add_cost(const StringName &p_attribute, float p_amount) {
	Dictionary cost;
	cost["attribute"] = p_attribute;
	cost["amount"] = p_amount;
	costs.append(cost);
}

bool AbilitySystemAbility::remove_cost(const StringName &p_attribute) {
	for (int i = 0; i < costs.size(); i++) {
		Dictionary cost = costs[i];
		StringName attr = cost["attribute"];
		if (cost.has("attribute") && attr == p_attribute) {
			costs.remove_at(i);
			return true;
		}
	}
	return false;
}

float AbilitySystemAbility::get_cost_amount(const StringName &p_attribute) const {
	for (int i = 0; i < costs.size(); i++) {
		Dictionary cost = costs[i];
		StringName attr = cost["attribute"];
		if (cost.has("attribute") && attr == p_attribute) {
			return cost["amount"];
		}
	}
	return 0.0f;
}

bool AbilitySystemAbility::can_afford_costs(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec) const {
	for (int i = 0; i < costs.size(); i++) {
		Dictionary cost = costs[i];
		if (cost.has("attribute") && cost.has("amount")) {
			StringName attribute = cost["attribute"];
			float amount = cost["amount"];

			if (use_custom_costs) {
				p_owner->GDVIRTUAL_CALL(_on_calculate_custom_magnitude, Variant(), -3 - i, amount);
			}

			float current_value = p_owner->get_attribute_value_by_tag(attribute);
			// Cost is treated as a positive value to subtract
			if (current_value < amount) {
				return false;
			}
		}
	}
	return true;
}

void AbilitySystemAbility::apply_costs(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec) const {
	for (int i = 0; i < costs.size(); i++) {
		Dictionary cost = costs[i];
		if (cost.has("attribute") && cost.has("amount")) {
			StringName attribute = cost["attribute"];
			float amount = cost["amount"];

			if (use_custom_costs) {
				p_owner->GDVIRTUAL_CALL(_on_calculate_custom_magnitude, Variant(), -3 - i, amount);
			}

			float base_value = p_owner->get_attribute_base_value_by_tag(attribute);
			// Subtract the cost magnitude from the base value
			p_owner->set_attribute_base_value_by_tag(attribute, base_value - amount);
		}
	}
}

AbilitySystemAbility::AbilitySystemAbility() {
}

AbilitySystemAbility::~AbilitySystemAbility() {
}

} // namespace godot
