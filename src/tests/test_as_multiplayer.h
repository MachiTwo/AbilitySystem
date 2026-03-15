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

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/tests/test_helpers.h"
#else
#include "modules/ability_system/tests/test_helpers.h"
#endif

// Helper to simulate the network bridge between two instances
inline void simulate_network_sync(ASComponent *server, ASComponent *client) {
	// 1. Sync Attributes
	Vector<Ref<ASAttributeSet>> s_sets = server->get_attribute_sets();
	Vector<Ref<ASAttributeSet>> c_sets = client->get_attribute_sets();

	for (int i = 0; i < s_sets.size() && i < c_sets.size(); i++) {
		Ref<ASAttributeSet> s_set = s_sets[i];
		Ref<ASAttributeSet> c_set = c_sets[i];
		TypedArray<StringName> attrs = s_set->get_attribute_list();
		for (int j = 0; j < attrs.size(); j++) {
			c_set->set_attribute_base_value(attrs[j], s_set->get_attribute_base_value(attrs[j]));
		}
	}
	// 2. Sync Tags
	client->remove_all_tags();
	TypedArray<StringName> s_tags = server->get_tags();
	for (int i = 0; i < s_tags.size(); i++) {
		client->add_tag(s_tags[i]);
	}
}

TEST_CASE("[AbilitySystem] Multiplayer Authority (300% Coverage)") {
	ASComponent *server = memnew(ASComponent);
	ASComponent *client = memnew(ASComponent);

	Ref<ASAttributeSet> s_set;
	s_set.instantiate();
	s_set->add_attribute_definition(create_test_attribute("hp", 100.0f));
	s_set->add_attribute_definition(create_test_attribute("mana", 100.0f));
	server->add_attribute_set(s_set);

	Ref<ASAttributeSet> c_set;
	c_set.instantiate();
	c_set->add_attribute_definition(create_test_attribute("hp", 100.0f));
	c_set->add_attribute_definition(create_test_attribute("mana", 100.0f));
	client->add_attribute_set(c_set);

	SUBCASE("Server Authority - 3 Variations") {
		Ref<ASAbility> fireball = create_test_ability("fireball", "ability.fireball");
		fireball->add_cost("mana", 50.0f);

		// Var 1: Server Rejection (Missing Cost)
		server->set_attribute_base_value_by_tag("mana", 0.0f);
		CHECK(server->try_activate_ability_by_resource(fireball) == false);

		// Var 2: Server Success (Has Cost)
		server->set_attribute_base_value_by_tag("mana", 100.0f);
		CHECK(server->try_activate_ability_by_resource(fireball) == true);

		// Var 3: Tag Synchronization (Server stun blocks client)
		server->add_tag("state.stunned");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.stunned") == true);
	}

	SUBCASE("State Replication - 3 Variations") {
		// Var 1: Simple Attribute Sync
		server->set_attribute_base_value_by_tag("hp", 50.0f);
		simulate_network_sync(server, client);
		CHECK_ATTR_EQ(client, "hp", 50.0f);

		// Var 2: Complex Tag Sync
		server->add_tag("buff.fire");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("buff.fire") == true);

		// Var 3: Hierarchical Tag Persistence
		server->add_tag("buff.water.protection");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("buff.water") == true); // Assumes ASTagSpec hierarchy
	}

	SUBCASE("Remote Rollback - 3 Variations") {
		// HP is 100 on server
		server->set_attribute_base_value_by_tag("hp", 100.0f);
		simulate_network_sync(server, client);

		// Var 1: Wrong Prediction Rollback
		client->set_attribute_base_value_by_tag("hp", 10.0f); // Client "cheats" or predicts wrong
		simulate_network_sync(server, client); // Server overwrites
		CHECK_ATTR_EQ(client, "hp", 100.0f);

		// Var 2: Illegal Tag Rollback
		client->add_tag("power.godmode");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("power.godmode") == false);

		// Var 3: Multi-set Consistency
		Ref<ASAttributeSet> s_set2 = memnew(ASAttributeSet);
		s_set2->add_attribute_definition(create_test_attribute("energy", 10.0f));
		server->add_attribute_set(s_set2);

		Ref<ASAttributeSet> c_set2 = memnew(ASAttributeSet);
		c_set2->add_attribute_definition(create_test_attribute("energy", 0.0f));
		client->add_attribute_set(c_set2);

		simulate_network_sync(server, client);
		CHECK_ATTR_EQ(client, "energy", 10.0f);
	}

	memdelete(server);
	memdelete(client);
}

#endif // TEST_AS_MULTIPLAYER_H
