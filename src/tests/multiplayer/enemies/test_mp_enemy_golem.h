/**************************************************************************/
/*  test_mp_enemy_golem.h                                                 */
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

#ifndef TEST_MP_ENEMY_GOLEM_H
#define TEST_MP_ENEMY_GOLEM_H

#include "../doctest.h"
#include "../mp_sync_helper.h"
#include "../test_mp_data.h"

// Golem Enemy
// High armor, resistant to crowd control
TEST_CASE("[AbilitySystem][MP] Enemy: Golem (300% Coverage)") {
	ASComponent *server = memnew(ASComponent);
	ASComponent *client = memnew(ASComponent);

	Ref<ASAttributeSet> s_set;
	s_set.instantiate();
	s_set->add_attribute_definition(MPData::get_singleton()->attr_hp);
	s_set->add_attribute_definition(MPData::get_singleton()->attr_armor);

	Ref<ASAttributeSet> c_set;
	c_set.instantiate();
	c_set->add_attribute_definition(MPData::get_singleton()->attr_hp);
	c_set->add_attribute_definition(MPData::get_singleton()->attr_armor);

	server->add_attribute_set(s_set);
	client->add_attribute_set(c_set);

	server->add_tag("enemy.golem");
	server->add_tag("immunity.cc"); // Custom tag for golem
	server->set_attribute_base_value("hp", 400.0f);
	server->set_attribute_base_value("armor", 200.0f);

	SUBCASE("Golem: Takes mitigated damage") {
		server->apply_effect_by_resource(MPData::get_singleton()->eff_base_damage);
		simulate_network_sync(server, client);
		CHECK(client->get_attribute_value_by_tag("hp") == 390.0f);
	}

	memdelete(server);
	memdelete(client);
}
#endif
