/**************************************************************************/
/*  test_mp_tags.h                                                        */
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

#ifndef TEST_MP_TAGS_H
#define TEST_MP_TAGS_H

#include "doctest.h"
#include "mp_sync_helper.h"
#include "src/scene/as_component.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AbilitySystem][MP] Tag Replication (300% Coverage)") {
	ASComponent *server = memnew(ASComponent);
	ASComponent *client = memnew(ASComponent);

	SUBCASE("Addition Replication - 3 Variations") {
		// Var 1: Single Tag
		server->add_tag("state.alive");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.alive") == true);

		// Var 2: Multiple Tags
		server->add_tag("buff.str");
		server->add_tag("buff.dex");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("buff.str") == true);
		CHECK(client->has_tag("buff.dex") == true);
		CHECK(client->get_owned_tags()->get_all_tags().size() == 3); // alive, str, dex

		// Var 3: Hierarchical Tags
		server->add_tag("state.stunned.heavy");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.stunned") == true); // Hierarchy check works on client
	}

	SUBCASE("Removal Replication - 3 Variations") {
		server->add_tag("state.burning");
		server->add_tag("state.frozen");
		server->add_tag("state.poisoned");
		simulate_network_sync(server, client);

		// Var 1: Single Removal
		server->remove_tag("state.burning");
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.burning") == false);
		CHECK(client->has_tag("state.frozen") == true);

		// Var 2: False Prediction Correction
		// Client assumes it removed frozen
		client->remove_tag("state.frozen");
		CHECK(client->has_tag("state.frozen") == false);
		// Server didn't approve, forces sync
		simulate_network_sync(server, client);
		CHECK(client->has_tag("state.frozen") == true);

		// Var 3: Full Clear
		server->get_owned_tags()->remove_all_tags();
		simulate_network_sync(server, client);
		CHECK(client->get_owned_tags()->get_all_tags().size() == 0);
	}

	SUBCASE("Conditional State Sync - 3 Variations") {
		// Used to test if the client locally resolves ANY/ALL conditions the same as server
		server->add_tag("weather.rain");
		server->add_tag("weather.cold");
		simulate_network_sync(server, client);

		TypedArray<StringName> any_cond;
		any_cond.push_back("weather.rain");
		any_cond.push_back("weather.hot");

		// Var 1: Client locally validates ANY correctly
		CHECK(client->get_owned_tags()->has_any_tags(any_cond) == true);

		TypedArray<StringName> all_cond;
		all_cond.push_back("weather.rain");
		all_cond.push_back("weather.hot");

		// Var 2: Client locally validates ALL correctly
		CHECK(client->get_owned_tags()->has_all_tags(all_cond) == false);

		// Var 3: Server state change updates client condition
		server->add_tag("weather.hot");
		simulate_network_sync(server, client);
		CHECK(client->get_owned_tags()->has_all_tags(all_cond) == true);
	}

	memdelete(server);
	memdelete(client);
}

#endif // TEST_MP_TAGS_H
