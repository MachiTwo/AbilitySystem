/**************************************************************************/
/*  test_gameplay_scenarios.h                                             */
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

#ifndef TEST_GAMEPLAY_SCENARIOS_H
#define TEST_GAMEPLAY_SCENARIOS_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/core/ability_system.h"
#include "src/core/as_ability_spec.h"
#include "src/core/as_effect_spec.h"
#include "src/resources/as_ability.h"
#include "src/resources/as_attribute_set.h"
#include "src/resources/as_effect.h"
#include "src/resources/as_package.h"
#include "src/scene/as_component.h"
#include "src/scene/as_delivery.h"
#include "src/tests/doctest.h"
#include "src/tests/test_tools.h"
#include <godot_cpp/classes/node.hpp>
#else
#include "core/object/object.h"
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/core/as_ability_spec.h"
#include "modules/ability_system/core/as_effect_spec.h"
#include "modules/ability_system/resources/as_ability.h"
#include "modules/ability_system/resources/as_attribute_set.h"
#include "modules/ability_system/resources/as_effect.h"
#include "modules/ability_system/resources/as_package.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/scene/as_delivery.h"
#include "modules/ability_system/tests/doctest.h"
#include "modules/ability_system/tests/test_tools.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

// Helper to fully initialize a character ASC
static ASComponent *create_mock_character(const StringName &p_name, float p_max_hp, float p_max_mana) {
	ASComponent *asc = memnew(ASComponent);
	asc->set_name(p_name);

	Ref<ASAttributeSet> attr_set;
	attr_set.instantiate();

	Ref<ASAttribute> hp;
	hp.instantiate();
	hp->set_attribute_name("Health");
	hp->set_min_value(0.0f);
	hp->set_max_value(p_max_hp);
	hp->set_default_value(p_max_hp);
	attr_set->add_attribute_definition(hp);

	Ref<ASAttribute> mp;
	mp.instantiate();
	mp->set_attribute_name("Mana");
	mp->set_min_value(0.0f);
	mp->set_max_value(p_max_mana);
	mp->set_default_value(p_max_mana);
	attr_set->add_attribute_definition(mp);

	asc->set_attribute_set(attr_set);
	// We simulate the ready phase
	return asc;
}

TEST_CASE("[Gameplay] The Tavern Encounter (NPC Dialogue & Potions)") {
	ASComponent *player = create_mock_character("Hero", 100.0f, 50.0f);
	ASComponent *merchant = create_mock_character("NPC", 50.0f, 0.0f);
	AbilitySystem::get_singleton()->register_tag("State.Paralyzed", AbilitySystem::TAG_TYPE_CONDITIONAL);

	SUBCASE("Dialogue Paralyzes Player - 3 Variations") {
		// Var 1: The NPC applies a strict paralyze state tag
		merchant->add_tag("State.Paralyzed", player);
		CHECK(player->has_tag("State.Paralyzed"));

		// Var 2: Attempting an action that blocks on paralyze
		Ref<ASAbility> attack_ability;
		attack_ability.instantiate();
		TypedArray<StringName> block_list;
		block_list.push_back(StringName("State.Paralyzed"));
		attack_ability->set_blocked_any_tags(block_list);

		player->unlock_ability(attack_ability);
		// Player tries to attack the NPC, but is blocked!
		bool activated = player->try_activate_ability_by_resource(attack_ability);
		CHECK_FALSE_MESSAGE(activated, "Player should be unable to attack while talking to NPC.");

		// Var 3: Dialogue ends, player is free and uses item
		merchant->remove_tag("State.Paralyzed", player);
		CHECK_FALSE(player->has_tag("State.Paralyzed"));
		bool can_attack_now = player->try_activate_ability_by_resource(attack_ability);
		CHECK_MESSAGE(can_attack_now, "Player should be able to attack after dialogue ends.");
	}

	SUBCASE("Healing Potion Consumption - 3 Variations") {
		// Var 1: Simulate the player took damage
		player->set_attribute_base_value("Health", 30.0f);

		// Var 2: Define a healing potion effect
		Ref<ASEffect> potion;
		potion.instantiate();
		potion->set_policy(ASEffect::POLICY_INSTANT);

		Dictionary mod;
		mod["attribute"] = StringName("Health");
		mod["operation"] = ASAttributeSet::MODIFIER_ADD;
		mod["magnitude"] = 50.0f; // Heals 50 HP
		TypedArray<Dictionary> modifiers;
		modifiers.push_back(mod);
		potion->set_modifiers(modifiers);

		// Var 3: Player drinks potion, values clamped properly
		player->apply_effect(potion);
		// Health was 30 + 50 = 80 HP
		CHECK(player->get_attribute_value("Health") == doctest::Approx(80.0f));
	}

	memdelete(player);
	memdelete(merchant);
}

TEST_CASE("[Gameplay] The Dungeon Trap & Horde Combat") {
	ASComponent *player = create_mock_character("Hero", 100.0f, 50.0f);
	ASComponent *goblin_1 = create_mock_character("Goblin_1", 30.0f, 0.0f);
	ASComponent *goblin_2 = create_mock_character("Goblin_2", 30.0f, 0.0f);
	ASComponent *trap = create_mock_character("TrapSystem", 999.0f, 999.0f); // Traps have ASCs to track hostility

	SUBCASE("Poison Dart Trap Delivery - 3 Variations") {
		// Var 1: Construct the poison logic
		Ref<ASEffect> poison;
		poison.instantiate();
		poison->set_policy(ASEffect::POLICY_DURATION);
		poison->set_duration(3.0f);
		poison->set_period(1.0f); // Ticks every 1 second

		Dictionary mod;
		mod["attribute"] = StringName("Health");
		mod["operation"] = ASAttributeSet::MODIFIER_ADD;
		mod["magnitude"] = -10.0f; // 10 damage per tick
		TypedArray<Dictionary> modifiers;
		modifiers.push_back(mod);
		poison->set_modifiers(modifiers);

		// Var 2: Package the poison into an ASDelivery
		Ref<ASPackage> dart_payload;
		dart_payload.instantiate();
		TypedArray<ASEffect> effects_payload;
		effects_payload.push_back(poison);
		dart_payload->set_effects(effects_payload);

		ASDelivery *delivery = memnew(ASDelivery);
		// Normally delivery is added to the tree, we simulate a hit
		delivery->_inject_package_into(player, dart_payload, trap);

		CHECK_MESSAGE(player->get_attribute_value("Health") == doctest::Approx(100.0f), "Damage is periodic, hasn't ticked yet.");

		// Var 3: Simulate Time Passing (Physics Process)
		Ref<ASEffectSpec> applied_poison = player->get_active_effects()[0];

		// 1 Tick (1 sec)
		applied_poison->tick(1.0f);
		CHECK(player->get_attribute_value("Health") == doctest::Approx(90.0f));

		// End of poison (3 secs total)
		applied_poison->tick(2.0f);
		CHECK(player->get_attribute_value("Health") == doctest::Approx(70.0f));
		CHECK_MESSAGE(applied_poison->is_expired(), "Poison should have expired after 3 seconds.");
	}

	SUBCASE("AoE Magical Explosion with Mana Cost - 3 Variations") {
		// Var 1: Setup AoE Explosion with 20 Mana Cost and 50 Damage
		Ref<ASAbility> explosion;
		explosion.instantiate();

		Ref<ASEffect> cost;
		cost.instantiate();
		cost->set_policy(ASEffect::POLICY_INSTANT);
		Dictionary cost_mod;
		cost_mod["attribute"] = StringName("Mana");
		cost_mod["operation"] = ASAttributeSet::MODIFIER_ADD;
		cost_mod["magnitude"] = -20.0f;
		TypedArray<Dictionary> costs;
		costs.push_back(cost_mod);
		cost->set_modifiers(costs);
		explosion->set_cost(cost);

		Ref<ASEffect> blast;
		blast.instantiate();
		blast->set_policy(ASEffect::POLICY_INSTANT);
		Dictionary blast_mod;
		blast_mod["attribute"] = StringName("Health");
		blast_mod["operation"] = ASAttributeSet::MODIFIER_ADD;
		blast_mod["magnitude"] = -50.0f;
		TypedArray<Dictionary> bursts;
		bursts.push_back(blast_mod);
		blast->set_modifiers(bursts);

		player->unlock_ability(explosion);

		// Var 2: Cost Evaluation & Failure (Not enough Mana)
		player->set_attribute_base_value("Mana", 10.0f);
		bool failure = player->try_activate_ability_by_resource(explosion);
		CHECK_FALSE_MESSAGE(failure, "Explosion should fail to activate without 20 Mana.");

		// Var 3: AoE Execute
		player->set_attribute_base_value("Mana", 50.0f);
		bool success = player->try_activate_ability_by_resource(explosion);
		CHECK_MESSAGE(success, "Explosion activated!");
		CHECK_MESSAGE(player->get_attribute_value("Mana") == doctest::Approx(30.0f), "Mana should be drained properly.");

		// Manually simulating the AoE impact on enemies since we aren't a physics engine
		goblin_1->apply_effect(blast);
		goblin_2->apply_effect(blast);

		CHECK_MESSAGE(goblin_1->get_attribute_value("Health") == doctest::Approx(0.0f), "Goblin 1 should be obliterated by 50 damage.");
		CHECK_MESSAGE(goblin_2->get_attribute_value("Health") == doctest::Approx(0.0f), "Goblin 2 should be obliterated by 50 damage.");
	}

	memdelete(player);
	memdelete(goblin_1);
	memdelete(goblin_2);
	memdelete(trap);
}

#endif // TEST_GAMEPLAY_SCENARIOS_H
