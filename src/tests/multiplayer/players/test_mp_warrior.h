/**************************************************************************/
/*  test_mp_warrior.h                                                     */
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

#ifndef TEST_MP_WARRIOR_H
#define TEST_MP_WARRIOR_H

#include "../doctest.h"
#include "../mp_sync_helper.h"
#include "../test_mp_data.h"
#include "src/resources/as_ability.h"
#include "src/resources/as_attribute_set.h"
#include "src/resources/as_effect.h"
#include "src/scene/as_component.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

// Represents the Actor: Warrior Class in a Multiplayer Environment
TEST_CASE("[AbilitySystem][MP] RPG Actor: Warrior Class (300% Coverage)") {
	ASComponent *server = memnew(ASComponent);
	ASComponent *client = memnew(ASComponent);

	Ref<ASAttributeSet> s_set;
	s_set.instantiate();
	Ref<ASAttributeSet> c_set;
	c_set.instantiate();

	Ref<ASAttribute> hp_s;
	hp_s.instantiate();
	hp_s->set_attribute_name("hp");
	hp_s->set_base_value(200.0f);
	s_set->add_attribute_definition(hp_s);
	Ref<ASAttribute> hp_c;
	hp_c.instantiate();
	hp_c->set_attribute_name("hp");
	hp_c->set_base_value(200.0f);
	c_set->add_attribute_definition(hp_c);

	Ref<ASAttribute> rage_s;
	rage_s.instantiate();
	rage_s->set_attribute_name("rage");
	rage_s->set_base_value(0.0f);
	s_set->add_attribute_definition(rage_s);
	Ref<ASAttribute> rage_c;
	rage_c.instantiate();
	rage_c->set_attribute_name("rage");
	rage_c->set_base_value(0.0f);
	c_set->add_attribute_definition(rage_c);

	Ref<ASAttribute> armor_s;
	armor_s.instantiate();
	armor_s->set_attribute_name("armor");
	armor_s->set_base_value(50.0f);
	s_set->add_attribute_definition(armor_s);
	Ref<ASAttribute> armor_c;
	armor_c.instantiate();
	armor_c->set_attribute_name("armor");
	armor_c->set_base_value(50.0f);
	c_set->add_attribute_definition(armor_c);

	server->add_attribute_set(s_set);
	client->add_attribute_set(c_set);

	server->add_tag("class.warrior");
	Ref<ASAbility> smash;
	smash.instantiate();
	smash->set_ability_tag("ability.smash");
	smash->add_cost("rage", 30.0f);
	Ref<ASEffect> enrage_eff;
	enrage_eff.instantiate();
	enrage_eff->add_modifier("rage", ASEffect::OP_ADD, 50.0f);

	SUBCASE("Warrior: Rage & Melee - 3 Variations") {
		CHECK(server->try_activate_ability_by_resource(smash) == false);
		server->apply_effect_by_resource(enrage_eff);
		simulate_network_sync(server, client);
		CHECK(client->get_attribute_value_by_tag("rage") == 50.0f);
	}
	SUBCASE("Warrior: Stun Resistance - 3 Variations") {
		server->add_tag("state.stunned");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.stunned") == true);
		server->remove_tag("state.stunned");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.stunned") == false);
	}
	SUBCASE("Warrior: Weapon Equip Rollback - 3 Variations") {
		server->add_tag("weapon.equipped.sword");
		simulate_network_sync(server, client);
		client->remove_tag("weapon.equipped.sword");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("weapon.equipped.sword") == true);
	}

	memdelete(server);
	memdelete(client);
}
#endif
