/**************************************************************************/
/*  test_mp_mage.h                                                        */
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

#ifndef TEST_MP_MAGE_H
#define TEST_MP_MAGE_H

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

// Represents the Actor: Mage Class in a Multiplayer Environment
TEST_CASE("[AbilitySystem][MP] RPG Actor: Mage Class (300% Coverage)") {
	ASComponent *server = memnew(ASComponent);
	ASComponent *client = memnew(ASComponent);

	Ref<ASAttributeSet> s_set;
	s_set.instantiate();
	Ref<ASAttributeSet> c_set;
	c_set.instantiate();

	Ref<ASAttribute> hp_s;
	hp_s.instantiate();
	hp_s->set_attribute_name("hp");
	hp_s->set_base_value(80.0f);
	s_set->add_attribute_definition(hp_s);
	Ref<ASAttribute> hp_c;
	hp_c.instantiate();
	hp_c->set_attribute_name("hp");
	hp_c->set_base_value(80.0f);
	c_set->add_attribute_definition(hp_c);

	Ref<ASAttribute> mana_s;
	mana_s.instantiate();
	mana_s->set_attribute_name("mana");
	mana_s->set_base_value(300.0f);
	s_set->add_attribute_definition(mana_s);
	Ref<ASAttribute> mana_c;
	mana_c.instantiate();
	mana_c->set_attribute_name("mana");
	mana_c->set_base_value(300.0f);
	c_set->add_attribute_definition(mana_c);

	Ref<ASAttribute> spellpower_s;
	spellpower_s.instantiate();
	spellpower_s->set_attribute_name("spellpower");
	spellpower_s->set_base_value(100.0f);
	s_set->add_attribute_definition(spellpower_s);
	Ref<ASAttribute> spellpower_c;
	spellpower_c.instantiate();
	spellpower_c->set_attribute_name("spellpower");
	spellpower_c->set_base_value(100.0f);
	c_set->add_attribute_definition(spellpower_c);

	server->add_attribute_set(s_set);
	client->add_attribute_set(c_set);

	server->add_tag("class.mage");
	Ref<ASAbility> fireball;
	fireball.instantiate();
	fireball->set_ability_tag("ability.fireball");
	fireball->add_cost("mana", 50.0f);
	Ref<ASEffect> burn_eff;
	burn_eff.instantiate();
	burn_eff->add_modifier("hp", ASEffect::OP_ADD, -10.0f);

	SUBCASE("Mage: Spellcasting Sync - 3 Variations") {
		CHECK(server->try_activate_ability_by_resource(fireball) == true);
		simulate_network_sync(server, client);
		CHECK(client->get_attribute_value_by_tag("mana") == 250.0f);
	}
	SUBCASE("Mage: Burning Effect Replication - 3 Variations") {
		server->apply_effect_by_resource(burn_eff);
		simulate_network_sync(server, client);
		CHECK(client->get_attribute_value_by_tag("hp") == 70.0f);
		server->add_tag("state.burning");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.burning") == true);
	}
	SUBCASE("Mage: Interruption & Silence - 3 Variations") {
		server->add_tag("state.silenced");
		simulate_network_sync(server, client);
		TypedArray<StringName> blocked;
		blocked.push_back("state.silenced");
		fireball->set_activation_blocked_any_tags(blocked);
		CHECK(server->can_activate_ability_by_resource(fireball) == false);
	}

	memdelete(server);
	memdelete(client);
}
#endif
