/**************************************************************************/
/*  as_bridge_prediction.cpp                                              */
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

#include "as_bridge_prediction.h"
#include "../limboai/bt/bt_player.h"
#include "../limboai/hsm/limbo_hsm.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include <godot_cpp/classes/time.hpp>
#else
#include "core/os/time.h"
#endif

void ASBridgePrediction::_bind_methods() {
	ClassDB::bind_method(D_METHOD("setup", "component", "ai_node"), &ASBridgePrediction::setup);
	ClassDB::bind_method(D_METHOD("capture_tick", "tick"), &ASBridgePrediction::capture_tick);
	ClassDB::bind_method(D_METHOD("rollback_to_tick", "tick"), &ASBridgePrediction::rollback_to_tick);

	ClassDB::bind_method(D_METHOD("set_max_history", "max"), &ASBridgePrediction::set_max_history);
	ClassDB::bind_method(D_METHOD("get_max_history"), &ASBridgePrediction::get_max_history);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_history"), "set_max_history", "get_max_history");
}

void ASBridgePrediction::setup(Node *p_component, Node *p_ai_node) {
	component_node = p_component;
	ai_node = p_ai_node;
}

void ASBridgePrediction::capture_tick(uint32_t p_tick) {
	if (!ai_node) {
		return;
	}

	// Use native prediction if available (LimboAI 2.0+)
	if (ai_node->has_method("capture_tick")) {
		ai_node->call("capture_tick", p_tick);
		return;
	}

	Dictionary state;

	// Try BTPlayer
	if (ai_node->is_class("BTPlayer")) {
		BTPlayer *player = Object::cast_to<BTPlayer>(ai_node);
		state = player->capture_state();
	}
	// Try LimboHSM
	else if (ai_node->is_class("LimboHSM")) {
		LimboHSM *hsm = Object::cast_to<LimboHSM>(ai_node);
		state = hsm->capture_state();
	}
	// Fallback for custom nodes or scripts
	else if (ai_node->has_method("capture_state")) {
		state = ai_node->call("capture_state");
	}

	ai_history[p_tick] = state;

	// Manage history size (cleanup old ticks)
	// We keep it simple: if size > max, remove oldest.
	// (In a real scenario, we'd use a circular buffer or sort keys)
	if (ai_history.size() > (int)max_history) {
		uint32_t oldest = p_tick - max_history;
		if (ai_history.has(oldest)) {
			ai_history.erase(oldest);
		}
	}
}

void ASBridgePrediction::rollback_to_tick(uint32_t p_tick) {
	if (!ai_node || !component_node) {
		return;
	}

	// 1. Rollback Ability System Component
	ASComponent *asc = Object::cast_to<ASComponent>(component_node);
	if (asc) {
		asc->rollback_to_tick(p_tick);
	}

	// 2. Rollback AI node
	if (ai_node->has_method("rollback_to_tick")) {
		ai_node->call("rollback_to_tick", p_tick);
	} else if (ai_history.has(p_tick)) {
		Dictionary state = ai_history[p_tick];

		if (ai_node->is_class("BTPlayer")) {
			BTPlayer *player = Object::cast_to<BTPlayer>(ai_node);
			player->restore_state(state);
		} else if (ai_node->is_class("LimboHSM")) {
			LimboHSM *hsm = Object::cast_to<LimboHSM>(ai_node);
			hsm->restore_state(state);
		} else if (ai_node->has_method("restore_state")) {
			ai_node->call("restore_state", state);
		}
	}

	// 3. Clean up future history (ticks > p_tick)
	// This is important because rolling back to T means T+1 haven't happened yet.
	Vector<uint32_t> to_remove;
	for (const auto &E : ai_history) {
		if (E.key > p_tick) {
			to_remove.push_back(E.key);
		}
	}
	for (int i = 0; i < to_remove.size(); i++) {
		ai_history.erase(to_remove[i]);
	}
}
