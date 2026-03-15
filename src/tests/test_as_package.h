/**************************************************************************/
/*  test_as_package.h                                                     */
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
#include "src/resources/as_cue.h"
#include "src/resources/as_effect.h"
#include "src/resources/as_package.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/resources/as_cue.h"
#include "modules/ability_system/resources/as_effect.h"
#include "modules/ability_system/resources/as_package.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AS] ASPackage Resource") {
	Ref<ASPackage> package;
	package.instantiate();

	SUBCASE("Basic Properties") {
		Ref<ASEffect> effect;
		effect.instantiate();
		effect->set_effect_tag("test.effect");

		TypedArray<ASEffect> effects;
		effects.push_back(effect);
		package->set_effects(effects);

		CHECK(package->get_effects().size() == 1);
		CHECK(Ref<ASEffect>(package->get_effects()[0])->get_effect_tag() == StringName("test.effect"));

		TypedArray<StringName> tags;
		tags.push_back(StringName("tag.test"));
		package->set_effect_tags(tags);
		CHECK(package->get_effect_tags().size() == 1);
		CHECK(StringName(package->get_effect_tags()[0]) == StringName("tag.test"));
	}

	SUBCASE("Cue Properties") {
		Ref<ASCue> cue;
		cue.instantiate();
		cue->set_cue_tag("test.cue");

		TypedArray<ASCue> cues;
		cues.push_back(cue);
		package->set_cues(cues);

		CHECK(package->get_cues().size() == 1);
		CHECK(Ref<ASCue>(package->get_cues()[0])->get_cue_tag() == StringName("test.cue"));
	}

	SUBCASE("Dynamic Manipulation") {
		// Effects
		Ref<ASEffect> e1 = memnew(ASEffect);
		package->add_effect(e1);
		CHECK(package->get_effects().size() == 1);
		package->remove_effect(e1);
		CHECK(package->get_effects().size() == 0);
		package->add_effect(e1);
		package->clear_effects();
		CHECK(package->get_effects().size() == 0);

		// Effect Tags
		package->add_effect_tag("state.poison");
		CHECK(package->get_effect_tags().size() == 1);
		package->remove_effect_tag("state.poison");
		CHECK(package->get_effect_tags().size() == 0);

		// Cues
		Ref<ASCue> c1 = memnew(ASCue);
		package->add_cue(c1);
		CHECK(package->get_cues().size() == 1);
		package->remove_cue(c1);
		CHECK(package->get_cues().size() == 0);
		package->add_cue(c1);
		package->clear_cues();
		CHECK(package->get_cues().size() == 0);

		// Cue Tags
		package->add_cue_tag("vfx.explosion");
		CHECK(package->get_cue_tags().size() == 1);
		package->remove_cue_tag("vfx.explosion");
		CHECK(package->get_cue_tags().size() == 0);
	}
}
