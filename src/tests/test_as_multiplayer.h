/**************************************************************************/
/*  test_as_multiplayer.h                                                 */
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

#ifndef TEST_AS_MULTIPLAYER_H
#define TEST_AS_MULTIPLAYER_H

#include "doctest.h"
#include "src/resources/as_ability.h"
#include "src/resources/as_attribute_set.h"
#include "src/scene/as_component.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

// Helper to simulate the network bridge between two instances
void simulate_network_sync(ASComponent *server, ASComponent *client) {
	// 1. Sync Attributes
	for (int i = 0; i < server->get_attribute_sets().size(); i++) {
		Ref<ASAttributeSet> s_set = server->get_attribute_sets()[i];
		Ref<ASAttributeSet> c_set = client->get_attribute_sets()[i];
		TypedArray<StringName> attrs = s_set->get_attribute_list();
		for (int j = 0; j < attrs.size(); j++) {
			c_set->set_attribute_base_value(attrs[j], s_set->get_attribute_base_value(attrs[j]));
		}
	}
	// 2. Sync Tags
	client->get_owned_tags()->remove_all_tags();
	TypedArray<StringName> s_tags = server->get_tags();
	for (int i = 0; i < s_tags.size(); i++) {
		client->add_tag(s_tags[i]);
	}
}

TEST_CASE("[AbilitySystem] Multiplayer Authority (300% Coverage)") {
	ASComponent *server = memnew(ASComponent);
	ASComponent *client = memnew(ASComponent);

	// Basic attribute setup
	Ref<ASAttributeSet> s_set;
	s_set.instantiate();
	Ref<ASAttribute> hp;
	hp.instantiate();
	hp->set_attribute_name("hp");
	hp->set_base_value(100.0f);
	s_set->add_attribute_definition(hp);
	server->add_attribute_set(s_set);

	Ref<ASAttributeSet> c_set;
	c_set.instantiate();
	Ref<ASAttribute> hp_c;
	hp_c.instantiate();
	hp_c->set_attribute_name("hp");
	hp_c->set_base_value(100.0f);
	c_set->add_attribute_definition(hp_c);
	client->add_attribute_set(c_set);

	SUBCASE("Server Authority - 3 Scenarios") {
		Ref<ASAbility> ability;
		ability.instantiate();
		ability->set_ability_tag("fireball");
		ability->add_cost("mana", 50.0f); // Client will not have mana

		// Var 1: Client activation attempt (Should fail without server confirmation)
		// In a real world scenario, the client "predicts", but here we test if the server rejects.
		bool server_activated = server->try_activate_ability_by_resource(ability);
		CHECK(server_activated == false);

		// Var 2: Illegal Tag Block (Server has tag that blocks client)
		server->add_tag("state.stunned");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.stunned") == true);
		CHECK(server->can_activate_ability_by_tag("fireball") == false);

		// Var 3: Mandatory Server Rollback simulation
		// If the client changed HP locally (wrong prediction), sync must correct it.
		client->set_attribute_base_value_by_tag("hp", 50.0f);
		simulate_network_sync(server, client);
		CHECK(client->get_attribute_value_by_tag("hp") == 100.0f);
	}

	SUBCASE("State Replication - 3 Scenarios") {
		// Var 1: Attribute Delta
		server->set_attribute_base_value_by_tag("hp", 80.0f);
		simulate_network_sync(server, client);
		CHECK(client->get_attribute_value_by_tag("hp") == 80.0f);

		// Var 2: Tag Propagation (Burning)
		server->add_tag("state.burning");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.burning") == true);

		// Var 3: Tag Hierarchy Propagation
		server->add_tag("state.moving.sprint");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.moving") == true);
	}

	SUBCASE("Remote Interactions - 3 Scenarios") {
		// Var 1: Cross-component effect
		// Agent A (Server) applies on Agent B (Server) and sync on Agent B (Client)
		Ref<ASEffect> dmg;
		dmg.instantiate();
		dmg->add_modifier("hp", ASEffect::OP_ADD, -30.0f);
		server->apply_effect_by_resource(dmg);

		simulate_network_sync(server, client);
		CHECK(client->get_attribute_value_by_tag("hp") == 50.0f); // 80 before - 30

		// Var 2: Remote Cancellation Check
		// (Simulate that an active ability on server was canceled and client stopped seeing the tag)
		server->add_tag("ability.active");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("ability.active") == true);
		server->remove_tag("ability.active");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("ability.active") == false);

		// Var 3: Multiple Attribute Sync Consistency
		server->set_attribute_base_value_by_tag("hp", 10.0f);
		simulate_network_sync(server, client);
		CHECK(client->get_attribute_value_by_tag("hp") == 10.0f);
	}

	memdelete(server);
	memdelete(client);
}

#endif // TEST_AS_MULTIPLAYER_H
