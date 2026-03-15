/**************************************************************************/
/*  test_as_cue_spec.h                                                    */
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

#ifndef TEST_AS_CUE_SPEC_H
#define TEST_AS_CUE_SPEC_H

#include "doctest.h"
#include "src/core/as_cue_spec.h"
#include "src/resources/as_cue.h"
#include "src/resources/as_cue_animation.h"
#include "src/resources/as_cue_audio.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AbilitySystem] ASCueSpec (300% Coverage)") {
	Ref<ASCueSpec> spec;
	spec.instantiate();

	SUBCASE("Data Context - 3 Variations") {
		// Var 1: Position
		Vector3 pos(10, 20, 30);
		spec->set_hit_position(pos);
		CHECK(Vector3(spec->get_hit_position()) == pos);

		// Var 2: Normal/Direction
		Vector3 normal(0, 1, 0);
		spec->set_hit_normal(normal);
		CHECK(Vector3(spec->get_hit_normal()) == normal);

		// Var 3: Generic Magnitude
		spec->set_level(5.5f);
		CHECK(spec->get_level() == 5.5f);
	}

	SUBCASE("Actor Context - 3 Variations") {
		Node *instigator = memnew(Node);
		Node *target = memnew(Node);
		Node *source = memnew(Node);

		// Var 1: Instigator
		spec->set_instigator(instigator);
		CHECK(spec->get_instigator() == instigator);

		// Var 2: Target
		spec->set_target(target);
		CHECK(spec->get_target() == target);

		// Var 3: Source Object
		spec->set_source_object(source);
		CHECK(spec->get_source_object() == source);

		memdelete(instigator);
		memdelete(target);
		memdelete(source);
	}

	SUBCASE("Cue Types (Polymorphism) - 3 Variations") {
		// Var 1: Audio Cue
		Ref<ASCueAudio> audio;
		audio.instantiate();
		audio->set_cue_tag("sfx.test");
		CHECK(audio->get_cue_tag() == StringName("sfx.test"));

		// Var 2: Animation Cue
		Ref<ASCueAnimation> anim;
		anim.instantiate();
		anim->set_animation_name("attack_01");
		CHECK(anim->get_animation_name() == String("attack_01"));

		// Var 3: Base Cue (Visual/Generic)
		Ref<ASCue> visual;
		visual.instantiate();
		visual->set_cue_tag("vfx.explosion");
		CHECK(visual->get_cue_tag() == StringName("vfx.explosion"));
	}
}

#endif // TEST_AS_CUE_SPEC_H
