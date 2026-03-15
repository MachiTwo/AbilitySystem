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

#ifndef TEST_AS_PACKAGE_H
#define TEST_AS_PACKAGE_H

#include "doctest.h"
#include "src/resources/as_cue.h"
#include "src/resources/as_effect.h"
#include "src/resources/as_package.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AbilitySystem] ASPackage (300% Coverage)") {
	Ref<ASPackage> package;
	package.instantiate();

	SUBCASE("Direct Assets - 3 Variations") {
		Ref<ASEffect> e1;
		e1.instantiate();
		Ref<ASEffect> e2;
		e2.instantiate();
		Ref<ASCue> c1;
		c1.instantiate();

		// Var 1: Add Effects
		package->add_effect(e1);
		package->add_effect(e2);
		CHECK(package->get_effects().size() == 2);

		// Var 2: Add Cues
		package->add_cue(c1);
		CHECK(package->get_cues().size() == 1);

		// Var 3: Bulk Clear
		package->clear_effects();
		package->clear_cues();
		CHECK(package->get_effects().size() == 0);
		CHECK(package->get_cues().size() == 0);
	}

	SUBCASE("Tagged Assets (Identity) - 3 Variations") {
		// Var 1: Effect Tags
		package->add_effect_tag("effect.fire");
		package->add_effect_tag("effect.burn");
		CHECK(package->get_effect_tags().size() == 2);

		// Var 2: Cue Tags
		package->add_cue_tag("vfx.spark");
		package->add_cue_tag("sfx.ignition");
		CHECK(package->get_cue_tags().size() == 2);

		// Var 3: Removal
		package->remove_effect_tag("effect.fire");
		package->remove_cue_tag("vfx.spark");
		CHECK(package->get_effect_tags().size() == 1);
		CHECK(package->get_cue_tags().size() == 1);
	}

	SUBCASE("State Tags - 3 Variations") {
		TypedArray<StringName> tags;
		tags.push_back("state.a");
		tags.push_back("state.b");
		tags.push_back("state.c");

		// Var 1: Bulk Set
		package->set_granted_tags(tags);
		CHECK(package->get_granted_tags().size() == 3);

		// Var 2: Individual Add
		package->add_granted_tag("state.d");
		CHECK(package->get_granted_tags().size() == 4);

		// Var 3: Match check
		CHECK(package->get_granted_tags().has(StringName("state.a")) == true);
	}
}

#endif // TEST_AS_PACKAGE_H
