/**************************************************************************/
/*  test_mp_data.h                                                        */
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

#ifndef TEST_MP_DATA_H
#define TEST_MP_DATA_H

#include "src/resources/as_ability.h"
#include "src/resources/as_attribute_set.h"
#include "src/resources/as_effect.h"
#include "src/scene/as_component.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

// ==============================================================================
// MPData: The Central Bank of Reusable Resources
//
// .tres files saved on disk in an RPG project) where all World tests (Actors)
// can consistently fetch Pointers, instantiating global data.
// ==============================================================================
class MPData {
private:
	static MPData *singleton;

public:
	static MPData *get_singleton() {
		if (!singleton) {
			singleton = memnew(MPData);
		}
		return singleton;
	}

	static void destroy() {
		if (singleton) {
			memdelete(singleton);
			singleton = nullptr;
		}
	}

	// =======================================================
	// 1. ATTRIBUTES (Definitions)
	// =======================================================
	// Showcases the raw statistics an actor is constituted of
	Ref<ASAttribute> attr_hp;
	Ref<ASAttribute> attr_mana;
	Ref<ASAttribute> attr_stamina;
	Ref<ASAttribute> attr_gold;
	Ref<ASAttribute> attr_armor;
	Ref<ASAttribute> attr_strength;
	Ref<ASAttribute> attr_evasion;

	// =======================================================
	// 2. EFFECTS (Buffs, Debuffs, Damage)
	// =======================================================
	Ref<ASEffect> eff_base_damage; // Flat neutral damage
	Ref<ASEffect> eff_base_heal; // Flat heal
	Ref<ASEffect> eff_poison; // Tick-based damage over time (DoT)
	Ref<ASEffect> eff_stun; // Crowd Control (CC)
	Ref<ASEffect> eff_armor_buff; // Temporary Armor Buff
	Ref<ASEffect> eff_evasion_buff; // Evasion Buff
	Ref<ASEffect> eff_frenzy; // Strength Buff with Armor Debuff (Rage)

	// =======================================================
	// 3. ABILITIES (Actions with Cost/Requirement and CD)
	// =======================================================
	Ref<ASAbility> abi_melee_attack; // Requires stamina
	Ref<ASAbility> abi_fireball; // Requires mana
	Ref<ASAbility> abi_heal; // Requires mana, long cooldown
	Ref<ASAbility> abi_stealth; // Activates stealth state
	Ref<ASAbility> abi_dash; // Dodge move, consumes stamina

	// =======================================================
	// 4. ITEMS / WEAPONS (Equipment Tags)
	// =======================================================
	// Generally, items provide tag combos ("weapon.sword", "weapon.equipped") and extra attributes

	MPData() {
		// ----------------------------------------------------------------------
		// ATTRIBUTE INITIALIZATION
		// ----------------------------------------------------------------------
		attr_hp.instantiate();
		attr_hp->set_attribute_name("hp");

		attr_mana.instantiate();
		attr_mana->set_attribute_name("mana");

		attr_stamina.instantiate();
		attr_stamina->set_attribute_name("stamina");

		attr_gold.instantiate();
		attr_gold->set_attribute_name("gold");

		attr_armor.instantiate();
		attr_armor->set_attribute_name("armor");

		attr_strength.instantiate();
		attr_strength->set_attribute_name("strength");

		attr_evasion.instantiate();
		attr_evasion->set_attribute_name("evasion");

		// ----------------------------------------------------------------------
		// EFFECT INITIALIZATION
		// ----------------------------------------------------------------------
		eff_base_damage.instantiate();
		eff_base_damage->add_modifier("hp", ASEffect::OP_ADD, -10.0f);

		eff_base_heal.instantiate();
		eff_base_heal->add_modifier("hp", ASEffect::OP_ADD, 20.0f);

		eff_poison.instantiate();
		eff_poison->add_modifier("hp", ASEffect::OP_ADD, -5.0f);
		eff_poison->set_duration_policy(ASEffect::DURATION);
		eff_poison->set_duration(10.0f);
		eff_poison->set_period(1.0f);
		TypedArray<StringName> poison_tags;
		poison_tags.push_back("state.poisoned");
		eff_poison->set_granted_tags(poison_tags);

		eff_stun.instantiate();
		eff_stun->set_duration_policy(ASEffect::DURATION);
		eff_stun->set_duration(3.0f);
		TypedArray<StringName> stun_tags;
		stun_tags.push_back("state.stunned");
		eff_stun->set_granted_tags(stun_tags);

		eff_armor_buff.instantiate();
		eff_armor_buff->add_modifier("armor", ASEffect::OP_ADD, 50.0f);
		eff_armor_buff->set_duration_policy(ASEffect::DURATION);
		eff_armor_buff->set_duration(15.0f);

		eff_frenzy.instantiate();
		eff_frenzy->add_modifier("strength", ASEffect::OP_ADD, 30.0f);
		eff_frenzy->add_modifier("armor", ASEffect::OP_ADD, -25.0f); // Sacrifices armor
		eff_frenzy->set_duration_policy(ASEffect::DURATION);
		eff_frenzy->set_duration(8.0f);
		TypedArray<StringName> frenzy_tags;
		frenzy_tags.push_back("state.frenzy");
		eff_frenzy->set_granted_tags(frenzy_tags);

		// ----------------------------------------------------------------------
		// ABILITY INITIALIZATION
		// ----------------------------------------------------------------------
		// MELEE ATTACK
		abi_melee_attack.instantiate();
		abi_melee_attack->set_ability_tag("ability.melee_attack");
		abi_melee_attack->add_cost("stamina", 10.0f);
		abi_melee_attack->set_cooldown_duration(1.0f);

		// FIREBALL
		abi_fireball.instantiate();
		abi_fireball->set_ability_tag("ability.fireball");
		abi_fireball->add_cost("mana", 25.0f);
		abi_fireball->set_cooldown_duration(3.0f);
		TypedArray<StringName> blocked_by;
		blocked_by.push_back("state.silenced");
		abi_fireball->set_activation_blocked_any_tags(blocked_by);

		// HEAL
		abi_heal.instantiate();
		abi_heal->set_ability_tag("ability.heal");
		abi_heal->add_cost("mana", 15.0f);
		abi_heal->set_cooldown_duration(5.0f);
		abi_heal->set_activation_blocked_any_tags(blocked_by); // Silence also blocks heal

		// STEALTH
		abi_stealth.instantiate();
		abi_stealth->set_ability_tag("ability.stealth");
		abi_stealth->add_cost("stamina", 20.0f);
		abi_stealth->set_cooldown_duration(10.0f);
		TypedArray<StringName> stealth_tags;
		stealth_tags.push_back("state.stealth");
		abi_stealth->set_activation_owned_tags(stealth_tags);

		// DASH (Lvl 1 Dash requires not being stunned)
		abi_dash.instantiate();
		abi_dash->set_ability_tag("ability.dash");
		abi_dash->add_cost("stamina", 15.0f);
		abi_dash->set_cooldown_duration(2.0f);
		TypedArray<StringName> dash_blocked;
		dash_blocked.push_back("state.stunned");
		dash_blocked.push_back("state.rooted");
		abi_dash->set_activation_blocked_any_tags(dash_blocked);
	}

	~MPData() {
		// Auto Release on destruction since Godot Ref handles refcount
	}
};

// Singleton instance init
MPData *MPData::singleton = nullptr;

#endif // TEST_MP_DATA_H
