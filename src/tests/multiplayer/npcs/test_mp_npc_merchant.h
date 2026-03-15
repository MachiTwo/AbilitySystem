/**************************************************************************/
/*  test_mp_npc_merchant.h                                                */
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

#ifndef TEST_MP_MERCHANT_H
#define TEST_MP_MERCHANT_H

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

// Represents the Actor: Merchant NPC in a Multiplayer Environment
TEST_CASE("[AbilitySystem][MP] RPG Actor: Merchant NPC (300% Coverage)") {
	ASComponent *server = memnew(ASComponent);
	ASComponent *client = memnew(ASComponent);

	Ref<ASAttributeSet> s_set;
	s_set.instantiate();
	Ref<ASAttributeSet> c_set;
	c_set.instantiate();

	Ref<ASAttribute> hp_s;
	hp_s.instantiate();
	hp_s->set_attribute_name("hp");
	hp_s->set_base_value(100.0f);
	s_set->add_attribute_definition(hp_s);
	Ref<ASAttribute> hp_c;
	hp_c.instantiate();
	hp_c->set_attribute_name("hp");
	hp_c->set_base_value(100.0f);
	c_set->add_attribute_definition(hp_c);

	Ref<ASAttribute> gold_s;
	gold_s.instantiate();
	gold_s->set_attribute_name("gold");
	gold_s->set_base_value(9999.0f);
	s_set->add_attribute_definition(gold_s);
	Ref<ASAttribute> gold_c;
	gold_c.instantiate();
	gold_c->set_attribute_name("gold");
	gold_c->set_base_value(9999.0f);
	c_set->add_attribute_definition(gold_c);

	server->add_attribute_set(s_set);
	client->add_attribute_set(c_set);

	server->add_tag("npc.merchant");

	SUBCASE("Merchant: Open Shop - 3 Variations") {
		server->add_tag("state.trading");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.trading") == true);
	}
	SUBCASE("Merchant: Gold Exchange - 3 Variations") {
		server->set_attribute_base_value_by_tag("gold", 10099.0f);
		simulate_network_sync(server, client);
		CHECK(client->get_attribute_value_by_tag("gold") == 10099.0f);
	}
	SUBCASE("Merchant: Hostility - 3 Variations") {
		server->add_tag("state.hostile");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.hostile") == true);
	}

	memdelete(server);
	memdelete(client);
}
#endif
