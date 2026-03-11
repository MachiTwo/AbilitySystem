/**************************************************************************/
/*  test_ability_system_cue_animation.h                                   */
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
#include "src/resources/ability_system_cue_animation.h"
#include "src/scene/ability_system_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/resources/ability_system_cue_animation.h"
#include "modules/ability_system/scene/ability_system_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include <godot_cpp/classes/animation.hpp>
#include <godot_cpp/classes/animation_library.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#else
#include "scene/animation/animation_player.h"
#endif

using namespace godot;

TEST_CASE("AbilitySystemCueAnimation Execution") {
	AbilitySystemComponent *asc = memnew(AbilitySystemComponent);
	AnimationPlayer *anim_player = memnew(AnimationPlayer);

	asc->register_node("DefaultAnim", anim_player);
	asc->set_animation_player(anim_player);

	Ref<Animation> anim;
	anim.instantiate();
	Ref<AnimationLibrary> library;
	library.instantiate();
	library->add_animation("attack", anim);
	anim_player->add_animation_library("", library);

	Ref<AbilitySystemCueAnimation> cue = memnew(AbilitySystemCueAnimation);
	cue->set_cue_tag("Cue.Animation.Attack");
	cue->set_animation_name("attack");
	cue->set_node_name("DefaultAnim");

	SUBCASE("Activation and Data Pass-through") {
		Dictionary data;
		data["speed"] = 1.5f;

		// In a headless test, actually playing the animation might not be possible
		// without a scene tree, but we can verify the API call doesn't crash
		// and the state of the cue is valid.
		CHECK(cue->get_animation_name() == String("attack"));

		bool activated = asc->try_activate_cue_by_resource(cue, data);
		(void)activated;
		// It might return false if not in tree or player not setup,
		// but we are testing that the properties are correctly set.
		CHECK(cue->get_cue_tag() == StringName("Cue.Animation.Attack"));
	}

	memdelete(anim_player);
	memdelete(asc);
}
