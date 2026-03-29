/**************************************************************************/
/*  test_as_bridge_tasks.h                                                */
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
#ifndef TEST_AS_BRIDGE_TASKS_H
#define TEST_AS_BRIDGE_TASKS_H

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/bridge/as_bridge_action_activate.h"
#include "src/bridge/as_bridge_condition_has_tag.h"
#include "src/resources/as_ability.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/bridge/as_bridge_action_activate.h"
#include "modules/ability_system/bridge/as_bridge_condition_has_tag.h"
#include "modules/ability_system/resources/as_ability.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("ASBridge - Tasks Integration") {
	Node *root = memnew(Node);
	ASComponent *asc = memnew(ASComponent);
	root->add_child(asc);

	Ref<ASAbility> fireball = memnew(ASAbility);
	fireball->set_ability_tag("Ability.Fireball");
	asc->unlock_ability_by_resource(fireball);

	SUBCASE("ActivateAbility Action - 3 Variations") {
		Ref<BTActionAS_ActivateAbility> action = memnew(BTActionAS_ActivateAbility);
		action->set_agent(asc);
		action->set_ability_tag("Ability.Fireball");

		// Var 1: Ativação bem-sucedida
		action->enter();
		CHECK(action->tick(0.1) == BT::SUCCESS);
		CHECK(asc->is_ability_active("Ability.Fireball"));

		// Var 2: Tentativa de ativação de habilidade bloqueada (sem recursos)
		action->set_ability_tag("Ability.NonExistent");
		action->enter();
		CHECK(action->tick(0.1) == BT::FAILURE);

		// Var 3: Ativação com cancelamento imediato
		action->set_ability_tag("Ability.Fireball");
		action->enter();
		asc->cancel_ability_by_tag("Ability.Fireball");
		CHECK(asc->is_ability_active("Ability.Fireball") == false);
	}

	SUBCASE("HasTag Condition - 3 Variations") {
		Ref<BTConditionAS_HasTag> cond = memnew(BTConditionAS_HasTag);
		cond->set_agent(asc);
		cond->set_tag("State.Burn");

		// Var 1: Condição falha (não tem a tag)
		CHECK(cond->tick(0.1) == BT::FAILURE);

		// Var 2: Condição passa (tag adicionada)
		asc->add_tag("State.Burn");
		CHECK(cond->tick(0.1) == BT::SUCCESS);

		// Var 3: Hierarquia de tags (Tag pai deve passar se tiver a filha)
		cond->set_tag("State"); // State.Burn herda de State no sistema hierárquico
		CHECK(cond->tick(0.1) == BT::SUCCESS);
	}

	memdelete(root);
}

#endif // TEST_AS_BRIDGE_TASKS_H
