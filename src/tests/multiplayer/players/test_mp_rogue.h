/**************************************************************************/
/*  test_mp_rogue.h                                                       */
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

#ifndef TEST_MP_ROGUE_H
#define TEST_MP_ROGUE_H

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

// Represents the Actor: Rogue Class in a Multiplayer Environment
TEST_CASE("[AbilitySystem][MP] RPG Actor: Rogue Class (300% Coverage)") {
	ASComponent *server = memnew(ASComponent);
	ASComponent *client = memnew(ASComponent);

	Ref<ASAttributeSet> s_set;
	s_set.instantiate();
	Ref<ASAttributeSet> c_set;
	c_set.instantiate();

	Ref<ASAttribute> hp_s;
	hp_s.instantiate();
	hp_s->set_attribute_name("hp");
	hp_s->set_base_value(120.0f);
	s_set->add_attribute_definition(hp_s);
	Ref<ASAttribute> hp_c;
	hp_c.instantiate();
	hp_c->set_attribute_name("hp");
	hp_c->set_base_value(120.0f);
	c_set->add_attribute_definition(hp_c);

	Ref<ASAttribute> energy_s;
	energy_s.instantiate();
	energy_s->set_attribute_name("energy");
	energy_s->set_base_value(100.0f);
	s_set->add_attribute_definition(energy_s);
	Ref<ASAttribute> energy_c;
	energy_c.instantiate();
	energy_c->set_attribute_name("energy");
	energy_c->set_base_value(100.0f);
	c_set->add_attribute_definition(energy_c);

	Ref<ASAttribute> agility_s;
	agility_s.instantiate();
	agility_s->set_attribute_name("agility");
	agility_s->set_base_value(80.0f);
	s_set->add_attribute_definition(agility_s);
	Ref<ASAttribute> agility_c;
	agility_c.instantiate();
	agility_c->set_attribute_name("agility");
	agility_c->set_base_value(80.0f);
	c_set->add_attribute_definition(agility_c);

	server->add_attribute_set(s_set);
	client->add_attribute_set(c_set);

	server->add_tag("class.rogue");
	Ref<ASAbility> stealth;
	stealth.instantiate();
	stealth->set_ability_tag("ability.stealth");
	Ref<ASAbility> backstab;
	backstab.instantiate();
	backstab->set_ability_tag("ability.backstab");
	backstab->add_cost("energy", 40.0f);
	TypedArray<StringName> req;
	req.push_back("state.stealth");
	backstab->set_activation_required_all_tags(req);

	SUBCASE("Rogue: Stealth Activation Sync - 3 Variations") {
		server->add_tag("state.stealth");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.stealth") == true);
	}
	SUBCASE("Rogue: Backstab Requirements - 3 Variations") {
		CHECK(server->try_activate_ability_by_resource(backstab) == false); // No stealth
		server->add_tag("state.stealth");
		CHECK(server->try_activate_ability_by_resource(backstab) == true); // Has stealth
		simulate_network_sync(server, client);
		CHECK(client->get_attribute_value_by_tag("energy") == 60.0f);
	}
	SUBCASE("Rogue: Stealth Break - 3 Variations") {
		server->add_tag("state.stealth");
		server->remove_tag("state.stealth");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.stealth") == false);
	}

	memdelete(server);
	memdelete(client);
}
#endif
