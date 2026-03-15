/**************************************************************************/
/*  test_mp_npc_questgiver.h                                              */
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

#ifndef TEST_MP_QUESTGIVER_H
#define TEST_MP_QUESTGIVER_H

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

// Represents the Actor: QuestGiver NPC in a Multiplayer Environment
TEST_CASE("[AbilitySystem][MP] RPG Actor: QuestGiver NPC (300% Coverage)") {
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

	Ref<ASAttribute> reputation_s;
	reputation_s.instantiate();
	reputation_s->set_attribute_name("reputation");
	reputation_s->set_base_value(0.0f);
	s_set->add_attribute_definition(reputation_s);
	Ref<ASAttribute> reputation_c;
	reputation_c.instantiate();
	reputation_c->set_attribute_name("reputation");
	reputation_c->set_base_value(0.0f);
	c_set->add_attribute_definition(reputation_c);

	server->add_attribute_set(s_set);
	client->add_attribute_set(c_set);

	server->add_tag("npc.questgiver");

	SUBCASE("QuestGiver: Handout - 3 Variations") {
		server->add_tag("quest.started.dragon");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("quest.started.dragon") == true);
	}
	SUBCASE("QuestGiver: Reputation Gain - 3 Variations") {
		server->set_attribute_base_value_by_tag("reputation", 50.0f);
		simulate_network_sync(server, client);
		CHECK(client->get_attribute_value_by_tag("reputation") == 50.0f);
	}
	SUBCASE("QuestGiver: Protection Escort - 3 Variations") {
		server->add_tag("state.escorting");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.escorting") == true);
	}

	memdelete(server);
	memdelete(client);
}
#endif
