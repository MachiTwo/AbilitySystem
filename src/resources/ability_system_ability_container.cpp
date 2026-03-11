/**************************************************************************/
/*  ability_system_ability_container.cpp                                  */
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
#include "src/resources/ability_system_ability_container.h"
#include "src/resources/ability_system_ability.h"
#include "src/resources/ability_system_attribute_set.h"
#include "src/resources/ability_system_cue.h"
#include "src/resources/ability_system_effect.h"
#else
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_ability_container.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"
#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#endif

namespace godot {

void AbilitySystemAbilityContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_abilities", "abilities"), &AbilitySystemAbilityContainer::set_abilities);
	ClassDB::bind_method(D_METHOD("get_abilities"), &AbilitySystemAbilityContainer::get_abilities);

	ClassDB::bind_method(D_METHOD("set_attribute_set", "attribute_set"), &AbilitySystemAbilityContainer::set_attribute_set);
	ClassDB::bind_method(D_METHOD("get_attribute_set"), &AbilitySystemAbilityContainer::get_attribute_set);

	ClassDB::bind_method(D_METHOD("set_effects", "effects"), &AbilitySystemAbilityContainer::set_effects);
	ClassDB::bind_method(D_METHOD("get_effects"), &AbilitySystemAbilityContainer::get_effects);

	ClassDB::bind_method(D_METHOD("set_cues", "cues"), &AbilitySystemAbilityContainer::set_cues);
	ClassDB::bind_method(D_METHOD("get_cues"), &AbilitySystemAbilityContainer::get_cues);

	ClassDB::bind_method(D_METHOD("has_ability", "ability"), &AbilitySystemAbilityContainer::has_ability);
	ClassDB::bind_method(D_METHOD("has_effect", "effect"), &AbilitySystemAbilityContainer::has_effect);
	ClassDB::bind_method(D_METHOD("has_cue", "tag"), &AbilitySystemAbilityContainer::has_cue);
	ClassDB::bind_method(D_METHOD("has_cue_resource", "cue"), &AbilitySystemAbilityContainer::has_cue_resource);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "abilities", PROPERTY_HINT_ARRAY_TYPE, "AbilitySystemAbility"), "set_abilities", "get_abilities");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "attribute_set", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemAttributeSet"), "set_attribute_set", "get_attribute_set");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "effects", PROPERTY_HINT_ARRAY_TYPE, "AbilitySystemEffect"), "set_effects", "get_effects");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "cues", PROPERTY_HINT_ARRAY_TYPE, "AbilitySystemCue"), "set_cues", "get_cues");
}

bool AbilitySystemAbilityContainer::has_ability(const Ref<AbilitySystemAbility> &p_ability) const {
	for (int i = 0; i < abilities.size(); i++) {
		if (abilities[i] == p_ability) {
			return true;
		}
	}
	// Check AttributeSet unlocked abilities as well
	if (attribute_set.is_valid()) {
		TypedArray<AbilitySystemAbility> unlocked = attribute_set->get_unlocked_abilities();
		for (int i = 0; i < unlocked.size(); i++) {
			if (unlocked[i] == p_ability) {
				return true;
			}
		}
	}
	return false;
}

bool AbilitySystemAbilityContainer::has_effect(const Ref<AbilitySystemEffect> &p_effect) const {
	for (int i = 0; i < effects.size(); i++) {
		if (effects[i] == p_effect) {
			return true;
		}
	}

	// Also check if any of the abilities use this effect as a cost, cooldown or main effect
	for (int i = 0; i < abilities.size(); i++) {
		Ref<AbilitySystemAbility> ability = abilities[i];
		if (ability.is_valid()) {
			if (ability->get_effect() == p_effect) {
				return true;
			}
			// Check costs
			TypedArray<Dictionary> costs = ability->get_costs();
			for (int j = 0; j < costs.size(); j++) {
				Dictionary cost = costs[j];
				if (cost.has("effect") && (Ref<AbilitySystemEffect>)cost["effect"] == p_effect) {
					return true;
				}
			}
		}
	}
	// Check AttributeSet unlocked abilities too
	if (attribute_set.is_valid()) {
		TypedArray<AbilitySystemAbility> unlocked = attribute_set->get_unlocked_abilities();
		for (int i = 0; i < unlocked.size(); i++) {
			Ref<AbilitySystemAbility> ability = unlocked[i];
			if (ability.is_valid()) {
				if (ability->get_effect() == p_effect) {
					return true;
				}
				// Check costs
				TypedArray<Dictionary> costs = ability->get_costs();
				for (int j = 0; j < costs.size(); j++) {
					Dictionary cost = costs[j];
					if (cost.has("effect") && (Ref<AbilitySystemEffect>)cost["effect"] == p_effect) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool AbilitySystemAbilityContainer::has_cue(const StringName &p_tag) const {
	for (int i = 0; i < cues.size(); i++) {
		Ref<AbilitySystemCue> cue = cues[i];
		if (cue.is_valid() && cue->get_cue_tag() == p_tag) {
			return true;
		}
	}
	return false;
}

bool AbilitySystemAbilityContainer::has_cue_resource(const Ref<AbilitySystemCue> &p_cue) const {
	for (int i = 0; i < cues.size(); i++) {
		if (cues[i] == p_cue) {
			return true;
		}
	}
	return false;
}

AbilitySystemAbilityContainer::AbilitySystemAbilityContainer() {
}

AbilitySystemAbilityContainer::~AbilitySystemAbilityContainer() {
}

} // namespace godot
