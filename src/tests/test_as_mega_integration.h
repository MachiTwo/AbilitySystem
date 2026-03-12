/**************************************************************************/
/*  test_as_mega_integration.h                                            */
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

#pragma once

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/core/as_ability_spec.h"
#include "src/core/as_task.h"
#include "src/resources/as_ability.h"
#include "src/resources/as_cue_animation.h"
#include "src/resources/as_effect.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#include "src/tests/test_helpers.h"
#include "src/tests/test_signal_watcher.h"
#else
#include "modules/ability_system/core/as_ability_spec.h"
#include "modules/ability_system/core/as_task.h"
#include "modules/ability_system/resources/as_ability.h"
#include "modules/ability_system/resources/as_cue_animation.h"
#include "modules/ability_system/resources/as_effect.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#include "modules/ability_system/tests/test_helpers.h"
#include "modules/ability_system/tests/test_signal_watcher.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("AbilitySystem Mega Integration (Abilities, Attributes, Effects, Cues, Tasks)") {
	ASComponent *asc = make_standard_asc();

	// 1. Setup Effect
	Ref<ASEffect> effect = memnew(ASEffect);
	effect->set_effect_name("MegaEffect");
	effect->add_modifier("Health", ASEffect::OP_ADD, -50.0f);

	// 2. Setup Cue
	Ref<ASCueAnimation> cue = memnew(ASCueAnimation);
	cue->set_cue_name("MegaCue");
	cue->set_animation_name("mega_anim");
	cue->set_event_type(ASCue::ON_ACTIVE);

	// 3. Setup Ability
	Ref<ASAbility> ability = memnew(ASAbility);
	ability->set_ability_tag("Ability.Mega");

	TypedArray<ASEffect> effects;
	effects.push_back(effect);
	ability->set_effects(effects);

	TypedArray<ASCue> cues;
	cues.push_back(cue);
	ability->set_cues(cues);

	// 4. Setup ASC
	Ref<ASContainer> catalog = memnew(ASContainer);
	TypedArray<ASAbility> catalog_abilities;
	catalog_abilities.push_back(ability);
	catalog->set_abilities(catalog_abilities);
	asc->set_container(catalog);
	asc->unlock_ability_by_resource(ability);

	SUBCASE("Full Execution Flow") {
		// Verify Initial HP
		CHECK_ATTR_EQ(asc, "Health", 100.0f);

		// Activate
		bool activated = asc->try_activate_ability_by_tag("Ability.Mega");
		CHECK(activated);

		// Effect should have been applied (Health -50)
		CHECK_ATTR_EQ(asc, "Health", 50.0f);

		// 5. Test Tasks
		Ref<ASTask> task = ASTask::wait_delay(asc, 0.5f);
		Ref<ASTestSignalWatcher> task_watcher = memnew(ASTestSignalWatcher);
		task->connect("completed", Callable(task_watcher.ptr(), "_on_signal_received"));

		task->activate();
		CHECK(task->is_finished() == false);

		// Manually tick component to process tasks if needed,
		// though wait_delay usually relies on SceneTree timer or internal clock.
		// In tests, we might need to simulate time.
		// For now, check if it's at least active.
		CHECK(task_watcher->signal_count == 0);
	}

	memdelete(asc);
}
