/**************************************************************************/
/*  test_as_bridge_state.h                                                */
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

// --- TEMPLATE OBRIGATÓRIO ---
#ifndef TEST_AS_BRIDGE_STATE_H
#define TEST_AS_BRIDGE_STATE_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/bridge/as_bridge_state.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/bridge/as_bridge_state.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("ASBridge - BridgeState Integration") {
	Node *root = memnew(Node);
	ASComponent *asc = memnew(ASComponent);
	asc->set_name("MainASC");
	root->add_child(asc);

	Ref<ASBridgeState> bridge = memnew(ASBridgeState);

	SUBCASE("ASC Resolution - 3 Variations") {
		// Var 1: Resolução automática via parent/agent
		bridge->initialize(asc);
		CHECK(bridge->get_actor_component() == asc);

		// Var 2: Resolução via alias registrado no componente
		asc->register_node("Self", asc);
		bridge->set_component_alias("Self");
		CHECK(bridge->get_actor_component() == asc);

		// Var 3: Resolução negativa (alias inexistente)
		bridge->set_component_alias("Invalid");
		// Deve retornar nullptr mas não crashar
		CHECK(bridge->get_actor_component() == nullptr);
	}

	SUBCASE("Blackboard Integration - 3 Variations") {
		Ref<Blackboard> bb = memnew(Blackboard);
		bridge->set_blackboard(bb);

		// Var 1: Sincronização de Blackboard
		CHECK(bridge->get_blackboard() == bb);

		// Var 2: Persistência de dados entre ticks da bridge
		bb->set_var("test_val", 42);
		CHECK((int)bridge->get_blackboard()->get_var("test_val") == 42);

		// Var 3: Troca de Blackboard em runtime
		Ref<Blackboard> bb2 = memnew(Blackboard);
		bridge->set_blackboard(bb2);
		CHECK(bridge->get_blackboard() == bb2);
	}

	memdelete(root);
}

#endif // TEST_AS_BRIDGE_STATE_H
