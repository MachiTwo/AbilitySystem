/**************************************************************************/
/*  mp_sync_helper.h                                                      */
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

#ifndef MP_SYNC_HELPER_H
#define MP_SYNC_HELPER_H

#include "src/resources/as_attribute_set.h"
#include "src/scene/as_component.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

// Helper to simulate the network bridge between server and client instances
inline void simulate_network_sync(ASComponent *server, ASComponent *client) {
	// 1. Sync Attributes (Base Values)
	for (int i = 0; i < server->get_attribute_sets().size(); i++) {
		if (i >= client->get_attribute_sets().size())
			break;
		Ref<ASAttributeSet> s_set = server->get_attribute_sets()[i];
		Ref<ASAttributeSet> c_set = client->get_attribute_sets()[i];
		TypedArray<StringName> attrs = s_set->get_attribute_list();
		for (int j = 0; j < attrs.size(); j++) {
			c_set->set_attribute_base_value(attrs[j], s_set->get_attribute_base_value(attrs[j]));
		}
	}
	// 2. Sync Tags (Full Override)
	client->get_owned_tags()->remove_all_tags();
	TypedArray<StringName> s_tags = server->get_tags();
	for (int i = 0; i < s_tags.size(); i++) {
		client->add_tag(s_tags[i]);
	}
}

#endif // MP_SYNC_HELPER_H
