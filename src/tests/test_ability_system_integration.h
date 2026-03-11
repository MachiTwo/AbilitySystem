/**************************************************************************/
/*  test_ability_system_integration.h                                     */
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

// test_ability_system_integration.h
#ifndef TEST_ABILITY_SYSTEM_INTEGRATION_H
#define TEST_ABILITY_SYSTEM_INTEGRATION_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/resources/ability_system_ability.h"
#include "src/resources/ability_system_attribute_set.h"
#include "src/resources/ability_system_effect.h"
#include "src/scene/ability_system_component.h"
#else
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/scene/ability_system_component.h"
#endif

#include "test_helpers.h"

using namespace godot;

TEST_CASE("High-Density Ability System Integration") {
	AbilitySystemComponent *asc = make_standard_asc();

	// Definitions
	const StringName tag_jump = "Ability.Jump";
	const StringName tag_walk = "Ability.Walk";
	const StringName tag_state_jumping = "State.Jumping";
	const StringName tag_state_walking = "State.Walking";
	const StringName tag_cue_jump = "Cue.Jump";
	const StringName tag_cue_walk = "Cue.Walk";

	// 1. Create a "Jump" ability that grants "State.Jumping" and has a Cue
	Ref<AbilitySystemEffect> jump_effect = make_duration_effect("JumpEffect", 1.0f, "Stamina", 0.0f, AbilitySystemEffect::OP_ADD, tag_state_jumping);
	Ref<AbilitySystemAbility> jump_ability = make_ability("Jump", tag_jump, jump_effect, 10.0f, "Stamina");
	jump_ability->set_duration_policy(AbilitySystemAbility::POLICY_DURATION);
	jump_ability->set_ability_duration(1.0f);

	// Add dummy cue
	Ref<AbilitySystemCue> jump_cue = memnew(AbilitySystemCue);
	jump_cue->set_cue_tag(tag_cue_jump);
	jump_cue->set_event_type(AbilitySystemCue::ON_ACTIVE);
	TypedArray<AbilitySystemCue> jump_cues;
	jump_cues.push_back(jump_cue);
	jump_ability->set_cues(jump_cues);

	// 2. Create a "Walk" ability
	Ref<AbilitySystemEffect> walk_effect = make_duration_effect("WalkEffect", 5.0f, "Stamina", 0.0f, AbilitySystemEffect::OP_ADD, tag_state_walking);
	Ref<AbilitySystemAbility> walk_ability = make_ability("Walk", tag_walk, walk_effect);
	walk_ability->set_duration_policy(AbilitySystemAbility::POLICY_INFINITE);

	Ref<AbilitySystemCue> walk_cue = memnew(AbilitySystemCue);
	walk_cue->set_cue_tag(tag_cue_walk);
	walk_cue->set_event_type(AbilitySystemCue::ON_ACTIVE);
	TypedArray<AbilitySystemCue> walk_cues;
	walk_cues.push_back(walk_cue);
	walk_ability->set_cues(walk_cues);

	// Setup Catalog
	Ref<AbilitySystemAbilityContainer> catalog = memnew(AbilitySystemAbilityContainer);
	TypedArray<AbilitySystemAbility> catalog_abilities;
	catalog_abilities.push_back(jump_ability);
	catalog_abilities.push_back(walk_ability);
	catalog->set_abilities(catalog_abilities);
	asc->set_ability_container(catalog);

	// Unlock abilities
	asc->unlock_ability_by_resource(jump_ability);
	asc->unlock_ability_by_resource(walk_ability);

	SUBCASE("Scenario: Activate, Validate, Cancel, Swap") {
		MESSAGE("Starting activation of Jump...");
		bool jump_ok = asc->try_activate_ability_by_tag(tag_jump);
		CHECK(jump_ok == true);

		MESSAGE("Validating Jump state...");
		CHECK_HAS_TAG(asc, tag_state_jumping);
		CHECK_HAS_EFFECT(asc, jump_effect);
		// Check stamina cost (init 80, cost 10)
		CHECK_ATTR_EQ(asc, "Stamina", 70.0f);

		MESSAGE("Canceling Jump and swapping to Walk...");
		asc->cancel_ability_by_tag(tag_jump);
		CHECK_NO_TAG(asc, tag_state_jumping);
		CHECK_NO_EFFECT(asc, jump_effect);

		bool walk_ok = asc->try_activate_ability_by_tag(tag_walk);
		CHECK(walk_ok == true);
		CHECK_HAS_TAG(asc, tag_state_walking);
		CHECK_HAS_EFFECT(asc, walk_effect);
	}

	SUBCASE("Scenario: Parallel Abilities (Jump + Walk)") {
		MESSAGE("Activating Walk...");
		bool walk_ok = asc->try_activate_ability_by_tag(tag_walk);
		CHECK(walk_ok == true);
		CHECK_HAS_TAG(asc, tag_state_walking);

		MESSAGE("Activating Jump while Walking...");
		bool jump_ok = asc->try_activate_ability_by_tag(tag_jump);
		CHECK(jump_ok == true);

		MESSAGE("Validating both states overlap...");
		CHECK_HAS_TAG(asc, tag_state_walking);
		CHECK_HAS_TAG(asc, tag_state_jumping);

		// Check stamina: 80 - 10 (jump cost) = 70.
		CHECK_ATTR_EQ(asc, "Stamina", 70.0f);

		MESSAGE("Validating effect stacking...");
		CHECK_HAS_EFFECT(asc, walk_effect);
		CHECK_HAS_EFFECT(asc, jump_effect);
	}

	SUBCASE("Scenario: Cost and Blocking") {
		asc->set_attribute_base_value_by_tag("Stamina", 5.0f);
		MESSAGE("Attempting Jump with low Stamina (cost=10)...");
		bool jump_fail = asc->try_activate_ability_by_tag(tag_jump);
		CHECK(jump_fail == false);
		CHECK_NO_TAG(asc, tag_state_jumping);

		MESSAGE("Adding a blocking tag: State.Dead");
		asc->add_tag("State.Dead");

		// Ability requirements: check if ability is blocked
		TypedArray<StringName> blocked;
		blocked.push_back(StringName("State.Dead"));
		jump_ability->set_activation_blocked_tags(blocked);

		asc->set_attribute_base_value_by_tag("Stamina", 100.0f);
		MESSAGE("Attempting Jump while Dead...");
		bool jump_blocked = asc->try_activate_ability_by_tag(tag_jump);
		CHECK(jump_blocked == false);
	}

	memdelete(asc);
}

TEST_CASE("High-Density Ability System RPG Flow" * doctest::skip(false)) {
	// Custom RPG Tags
	const StringName tag_state_talking = "State.Talking";
	const StringName tag_state_dead = "State.Dead";
	const StringName tag_state_stunned = "State.Stunned";
	const StringName tag_ability_melee = "Ability.Melee";
	const StringName tag_ability_fireball = "Ability.Fireball";
	const StringName tag_ability_heal = "Ability.Heal";
	const StringName tag_ability_talk = "Ability.Talk";

	// Shared Effects/Abilities
	Ref<AbilitySystemEffect> melee_damage = make_instant_effect("MeleeDamage", "Health", -25.0f);
	Ref<AbilitySystemAbility> melee_ability = make_ability("MeleeAttack", tag_ability_melee, melee_damage, 10.0f, "Stamina");
	TypedArray<StringName> melee_blocked;
	melee_blocked.push_back(tag_state_talking);
	melee_blocked.push_back(tag_state_dead);
	melee_ability->set_activation_blocked_tags(melee_blocked);

	Ref<AbilitySystemEffect> burn_effect = memnew(AbilitySystemEffect);
	burn_effect->set_effect_name("Burning");
	burn_effect->set_duration_policy(AbilitySystemEffect::POLICY_DURATION);
	burn_effect->set_duration_magnitude(5.1f);
	burn_effect->set_period(1.0f);
	burn_effect->add_modifier("Health", AbilitySystemEffect::OP_ADD, -5.0f);

	Ref<AbilitySystemAbility> fireball_ability = make_ability("Fireball", tag_ability_fireball, nullptr, 30.0f, "Mana");
	fireball_ability->set_cooldown_duration(5.0f);

	Ref<AbilitySystemEffect> heal_effect = make_instant_effect("HealEffect", "Health", 50.0f);
	Ref<AbilitySystemAbility> heal_ability = make_ability("Heal", tag_ability_heal, heal_effect, 40.0f, "Mana");
	heal_ability->set_cooldown_duration(10.0f);

	Ref<AbilitySystemEffect> talk_state_effect = memnew(AbilitySystemEffect);
	talk_state_effect->set_effect_name("TalkState");
	talk_state_effect->set_duration_policy(AbilitySystemEffect::POLICY_INFINITE);
	TypedArray<StringName> talk_granted_tags;
	talk_granted_tags.push_back(tag_state_talking);
	talk_state_effect->set_granted_tags(talk_granted_tags);
	Ref<AbilitySystemAbility> talk_ability = make_ability("Talk", tag_ability_talk, talk_state_effect);
	talk_ability->set_duration_policy(AbilitySystemAbility::POLICY_INFINITE);

	// Shared Catalog
	Ref<AbilitySystemAbilityContainer> shared_catalog = memnew(AbilitySystemAbilityContainer);
	TypedArray<AbilitySystemAbility> catalog_abilities;
	catalog_abilities.push_back(melee_ability);
	catalog_abilities.push_back(fireball_ability);
	catalog_abilities.push_back(heal_ability);
	catalog_abilities.push_back(talk_ability);
	shared_catalog->set_abilities(catalog_abilities);

	SUBCASE("Phase 2: Dialog and Action Blocking") {
		AbilitySystemComponent *player_asc = make_standard_asc(100.0f, 100.0f, 100.0f);
		player_asc->set_ability_container(shared_catalog);
		player_asc->unlock_ability_by_resource(melee_ability);
		player_asc->unlock_ability_by_resource(talk_ability);

		MESSAGE("Simulating Dialog with NPC...");
		player_asc->try_activate_ability_by_tag(tag_ability_talk);
		CHECK_HAS_TAG(player_asc, tag_state_talking);

		MESSAGE("Attempting to attack while talking (Should be BLOCKED)...");
		bool melee_fail = player_asc->try_activate_ability_by_tag(tag_ability_melee);
		CHECK(melee_fail == false);
		CHECK_ATTR_EQ(player_asc, "Stamina", 100.0f);

		MESSAGE("Ending dialog and retrying attack...");
		player_asc->cancel_ability_by_tag(tag_ability_talk);
		CHECK_NO_TAG(player_asc, tag_state_talking);

		bool melee_ok = player_asc->try_activate_ability_by_tag(tag_ability_melee);
		CHECK(melee_ok == true);
		CHECK_ATTR_EQ(player_asc, "Stamina", 90.0f);

		memdelete(player_asc);
	}

	SUBCASE("Phase 3: Fireballs, Burns and Cooldowns") {
		AbilitySystemComponent *player_asc = make_standard_asc(100.0f, 100.0f, 100.0f);
		AbilitySystemComponent *enemy_asc = make_standard_asc(200.0f, 0.0f, 0.0f);
		player_asc->set_ability_container(shared_catalog);
		player_asc->unlock_ability_by_resource(fireball_ability);

		MESSAGE("Player casts Fireball on Enemy...");
		player_asc->try_activate_ability_by_tag(tag_ability_fireball);
		CHECK_ATTR_EQ(player_asc, "Mana", 70.0f);

		Ref<AbilitySystemEffectSpec> spec = player_asc->make_outgoing_spec(burn_effect);
		player_asc->apply_effect_spec_to_target(spec, enemy_asc);
		CHECK_HAS_EFFECT(enemy_asc, burn_effect);

		MESSAGE("Verifying Burn damage over time (Ticking)...");
		enemy_asc->tick(1.1f); // Tick 1
		CHECK_ATTR_EQ(enemy_asc, "Health", 195.0f);
		enemy_asc->tick(1.0f); // Tick 2
		CHECK_ATTR_EQ(enemy_asc, "Health", 190.0f);

		MESSAGE("Checking Cooldown Management...");
		bool fireball_cooldown = player_asc->try_activate_ability_by_tag(tag_ability_fireball);
		CHECK(fireball_cooldown == false);

		player_asc->tick(5.1f);
		bool fireball_ready = player_asc->try_activate_ability_by_tag(tag_ability_fireball);
		CHECK(fireball_ready == true);

		memdelete(player_asc);
		memdelete(enemy_asc);
	}

	SUBCASE("Phase 4: Stun & Interruption") {
		AbilitySystemComponent *player_asc = make_standard_asc(100.0f, 100.0f, 100.0f);
		player_asc->set_ability_container(shared_catalog);
		player_asc->unlock_ability_by_resource(heal_ability);
		player_asc->unlock_ability_by_resource(melee_ability);

		MESSAGE("Player starts healing...");
		player_asc->try_activate_ability_by_tag(tag_ability_heal);
		CHECK_ATTR_EQ(player_asc, "Mana", 60.0f);

		MESSAGE("Enemy applies STUN to player...");
		player_asc->add_tag(tag_state_stunned);

		MESSAGE("Attempting Melee while stunned...");
		TypedArray<StringName> blocked = melee_ability->get_activation_blocked_tags();
		blocked.push_back(tag_state_stunned);
		melee_ability->set_activation_blocked_tags(blocked);

		bool melee_fail = player_asc->try_activate_ability_by_tag(tag_ability_melee);
		CHECK(melee_fail == false);

		memdelete(player_asc);
	}

	SUBCASE("Phase 5: Death Resolution") {
		AbilitySystemComponent *player_asc = make_standard_asc(100.0f, 100.0f, 100.0f);
		AbilitySystemComponent *enemy_asc = make_standard_asc(200.0f, 0.0f, 0.0f);
		player_asc->set_ability_container(shared_catalog);
		player_asc->unlock_ability_by_resource(melee_ability);

		MESSAGE("Reducing Enemy Health to zero...");
		enemy_asc->set_attribute_base_value_by_tag("Health", 0.0f);
		enemy_asc->add_tag(tag_state_dead);
		CHECK_HAS_TAG(enemy_asc, tag_state_dead);

		MESSAGE("Verifying actions on dead target...");
		bool apply_fail = player_asc->can_activate_ability_by_tag(tag_ability_melee);
		CHECK(apply_fail == true);

		MESSAGE("Killing player and verifying total block...");
		player_asc->add_tag(tag_state_dead);
		bool player_attack_fail = player_asc->try_activate_ability_by_tag(tag_ability_melee);
		CHECK(player_attack_fail == false);

		memdelete(player_asc);
		memdelete(enemy_asc);
	}

	SUBCASE("Phase 6: Ability Duration & Auto-cleanup") {
		AbilitySystemComponent *asc = make_standard_asc(100.0f, 100.0f, 100.0f);

		Ref<AbilitySystemAbility> timed_ability = memnew(AbilitySystemAbility);
		timed_ability->set_ability_tag("Ability.Timed");
		timed_ability->set_duration_policy(AbilitySystemAbility::POLICY_DURATION);
		timed_ability->set_ability_duration(1.0f);

		TypedArray<StringName> owned;
		owned.push_back("State.Active");
		timed_ability->set_activation_owned_tags(owned);

		// Setup Catalog for local ability
		Ref<AbilitySystemAbilityContainer> local_catalog = memnew(AbilitySystemAbilityContainer);
		TypedArray<AbilitySystemAbility> local_abilities;
		local_abilities.push_back(timed_ability);
		local_catalog->set_abilities(local_abilities);
		asc->set_ability_container(local_catalog);

		asc->unlock_ability_by_resource(timed_ability);
		asc->try_activate_ability_by_tag("Ability.Timed");

		CHECK_HAS_TAG(asc, "State.Active");

		// Simulate 0.5s pass
		asc->tick(0.5f);
		CHECK_HAS_TAG(asc, "State.Active");

		// Simulate another 0.6s pass (total 1.1s)
		asc->tick(0.6f);
		CHECK_FALSE(asc->has_tag("State.Active"));

		memdelete(asc);
	}

	SUBCASE("Phase 7: Attribute Requirements (Non-consuming)") {
		AbilitySystemComponent *asc = make_standard_asc(100.0f, 100.0f, 100.0f); // Health, Mana, Stamina

		Ref<AbilitySystemAbility> req_ability = memnew(AbilitySystemAbility);
		req_ability->set_ability_tag("Ability.Required");
		req_ability->add_requirement("Mana", 150.0f); // Current is 100.0f

		// Setup Catalog for local ability
		Ref<AbilitySystemAbilityContainer> local_catalog = memnew(AbilitySystemAbilityContainer);
		TypedArray<AbilitySystemAbility> local_abilities;
		local_abilities.push_back(req_ability);
		local_catalog->set_abilities(local_abilities);
		asc->set_ability_container(local_catalog);

		asc->unlock_ability_by_resource(req_ability);

		MESSAGE("Verifying requirement block...");
		bool activated = asc->try_activate_ability_by_tag("Ability.Required");
		CHECK_FALSE(activated);

		MESSAGE("Increasing attribute to meet requirement...");
		asc->set_attribute_base_value_by_tag("Mana", 200.0f);
		activated = asc->try_activate_ability_by_tag("Ability.Required");
		CHECK(activated);

		MESSAGE("Verifying attribute was not consumed by requirement...");
		CHECK_ATTR_EQ(asc, "Mana", 200.0f);

		memdelete(asc);
	}
}

#endif // TEST_ABILITY_SYSTEM_INTEGRATION_H
