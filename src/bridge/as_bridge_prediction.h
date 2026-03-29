/**************************************************************************/
/*  as_bridge_prediction.h                                                */
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

#ifndef AS_BRIDGE_PREDICTION_H
#define AS_BRIDGE_PREDICTION_H

#include "../compat/limboai_bt.h"
#include "../compat/limboai_hsm.h"
#include "../scene/as_component.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/dictionary.hpp>
using namespace godot;
#else
#include "core/templates/hash_map.h"
#include "core/variant/dictionary.h"
#include "scene/main/node.h"
#endif

/**
 * ASBridgePrediction - Synchronized Rollback for AS and LimboAI
 *
 * This node manages a combined history of Ability System Component and AI (BT/HSM) state.
 * It allows the entire character behavior logic to rollback to a specific tick.
 */
class ASBridgePrediction : public Node {
	GDCLASS(ASBridgePrediction, Node);

private:
	Node *component_node = nullptr;
	Node *ai_node = nullptr;

	HashMap<uint32_t, Dictionary> ai_history;
	uint32_t max_history = 128;

protected:
	static void _bind_methods();

public:
	void setup(Node *p_component, Node *p_ai_node);
	void capture_tick(uint32_t p_tick);
	void rollback_to_tick(uint32_t p_tick);

	void set_max_history(uint32_t p_max) { max_history = p_max; }
	uint32_t get_max_history() const { return max_history; }

	ASBridgePrediction() = default;
	virtual ~ASBridgePrediction() = default;
};

#endif // AS_BRIDGE_PREDICTION_H
