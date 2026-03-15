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

#ifndef TEST_AS_COMPONENT_H
#define TEST_AS_COMPONENT_H

#include "doctest.h"
#include "src/resources/as_ability.h"
#include "src/resources/as_attribute.h"
#include "src/resources/as_attribute_set.h"
#include "src/resources/as_container.h"
#include "src/scene/as_component.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AbilitySystem] ASComponent (300% Coverage)") {
	ASComponent *asc = memnew(ASComponent);
	Ref<ASAttributeSet> set;
	set.instantiate();

	Ref<ASAttribute> hp;
	hp.instantiate();
	hp->set_attribute_name("hp");
	hp->set_base_value(100.0f);
	Ref<ASAttribute> mp;
	mp.instantiate();
	mp->set_attribute_name("mp");
	mp->set_base_value(50.0f);
	Ref<ASAttribute> lv;
	lv.instantiate();
	lv->set_attribute_name("lv");
	lv->set_base_value(1.0f);

	set->add_attribute_definition(hp);
	set->add_attribute_definition(mp);
	set->add_attribute_definition(lv);
	asc->add_attribute_set(set);

	SUBCASE("Ability Activation Failures - 3 Variations") {
		Ref<ASAbility> ability;
		ability.instantiate();
		ability->set_ability_tag("attack");
		ability->add_cost("mp", 100.0f); // Higher than current
		ability->set_cooldown_duration(5.0f);

		Ref<ASContainer> container;
		container.instantiate();
		TypedArray<ASAbility> abilities;
		abilities.push_back(ability);
		container->set_abilities(abilities);
		asc->set_container(container);

		// Var 1: Locked Failure
		CHECK(asc->try_activate_ability_by_tag("attack") == false);

		// Var 2: Cost Failure
		asc->unlock_ability_by_tag("attack");
		CHECK(asc->try_activate_ability_by_tag("attack") == false);

		// Var 3: Cooldown Failure (Force start cooldown first)
		asc->set_attribute_base_value_by_tag("mp", 200.0f);
		asc->start_cooldown("attack", 5.0f, TypedArray<StringName>());
		CHECK(asc->try_activate_ability_by_tag("attack") == false);
	}

	SUBCASE("Effect Duration Policies - 3 Variations") {
		// Var 1: Instant
		Ref<ASEffect> inst;
		inst.instantiate();
		inst->set_duration_policy(ASEffect::POLICY_INSTANT);
		inst->add_modifier("hp", ASEffect::OP_ADD, -10.0f);
		asc->apply_effect_by_resource(inst);
		CHECK(asc->get_attribute_value_by_tag("hp") == 90.0f);

		// Var 2: Duration
		Ref<ASEffect> dur;
		dur.instantiate();
		dur->set_duration_policy(ASEffect::POLICY_DURATION);
		dur->set_duration_magnitude(2.0f);
		dur->add_modifier("hp", ASEffect::OP_ADD, 20.0f);
		asc->apply_effect_by_resource(dur);
		CHECK(asc->get_attribute_value_by_tag("hp") == 110.0f);
		asc->tick(2.1f);
		CHECK(asc->get_attribute_value_by_tag("hp") == 90.0f);

		// Var 3: Infinite
		Ref<ASEffect> inf;
		inf.instantiate();
		inf->set_duration_policy(ASEffect::POLICY_INFINITE);
		inf->add_modifier("hp", ASEffect::OP_ADD, 5.0f);
		asc->apply_effect_by_resource(inf);
		CHECK(asc->get_attribute_value_by_tag("hp") == 95.0f);
		asc->remove_effect_by_resource(inf);
		CHECK(asc->get_attribute_value_by_tag("hp") == 90.0f);
	}

	SUBCASE("Node Registration & Context - 3 Variations") {
		Node *n1 = memnew(Node);
		Node *n2 = memnew(Node);
		Node *n3 = memnew(Node);

		// Var 1: Animation Player
		asc->set_animation_player(n1);
		CHECK(asc->get_animation_player() == n1);

		// Var 2: Audio Player
		asc->set_audio_player(n2);
		CHECK(asc->get_audio_player() == n2);

		// Var 3: Custom Named Node
		asc->register_node("camera", n3);
		CHECK(asc->get_node_ptr("camera") == n3);

		memdelete(n1);
		memdelete(n2);
		memdelete(n3);
	}

	memdelete(asc);
}

#endif // TEST_AS_COMPONENT_H
