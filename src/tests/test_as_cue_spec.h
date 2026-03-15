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

#pragma once

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/core/as_cue_spec.h"
#include "src/resources/as_cue.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/core/as_cue_spec.h"
#include "modules/ability_system/resources/as_cue.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("ASCueSpec Data Handling") {
	Ref<ASCueSpec> spec = memnew(ASCueSpec);

	SUBCASE("Basic data") {
		spec->set_level(10.0f);
		CHECK(spec->get_level() == 10.0f);

		Vector3 pos(1, 2, 3);
		spec->set_hit_position(pos);
		CHECK(spec->get_hit_position() == Variant(pos));
	}
}

TEST_CASE("ASCue API") {
	Ref<ASCue> cue = memnew(ASCue);

	SUBCASE("Activation Required All Tags (AND) - 3 Variations") {
		// Var 1: Round-trip — verify content by name
		TypedArray<StringName> req;
		req.push_back("State.Grounded");
		req.push_back("State.Alive");
		cue->set_activation_required_all_tags(req);
		CHECK(cue->get_activation_required_all_tags().size() == 2);
		CHECK(StringName(cue->get_activation_required_all_tags()[0]) == StringName("State.Grounded"));
		CHECK(StringName(cue->get_activation_required_all_tags()[1]) == StringName("State.Alive"));

		// Var 2: Replace — new list replaces old entirely
		TypedArray<StringName> req2;
		req2.push_back("State.InCombat");
		cue->set_activation_required_all_tags(req2);
		CHECK(cue->get_activation_required_all_tags().size() == 1);
		CHECK(StringName(cue->get_activation_required_all_tags()[0]) == StringName("State.InCombat"));

		// Var 3: Empty — no ALL requirement (always passes AND check)
		cue->set_activation_required_all_tags(TypedArray<StringName>());
		CHECK(cue->get_activation_required_all_tags().size() == 0);
	}

	SUBCASE("Activation Required Any Tags (OR) - 3 Variations") {
		// Var 1: Round-trip — triple option list
		TypedArray<StringName> req;
		req.push_back("buff.fire");
		req.push_back("buff.water");
		req.push_back("buff.earth");
		cue->set_activation_required_any_tags(req);
		CHECK(cue->get_activation_required_any_tags().size() == 3);
		CHECK(StringName(cue->get_activation_required_any_tags()[0]) == StringName("buff.fire"));

		// Var 2: Replace — narrow down to single option
		TypedArray<StringName> req2;
		req2.push_back("buff.fire");
		cue->set_activation_required_any_tags(req2);
		CHECK(cue->get_activation_required_any_tags().size() == 1);
		CHECK(StringName(cue->get_activation_required_any_tags()[0]) == StringName("buff.fire"));

		// Var 3: Empty — no OR requirement (always passes OR check)
		cue->set_activation_required_any_tags(TypedArray<StringName>());
		CHECK(cue->get_activation_required_any_tags().size() == 0);
	}

	SUBCASE("Activation Blocked Any Tags (OR) - 3 Variations") {
		// Var 1: Round-trip — two blockers, verify by name
		TypedArray<StringName> block;
		block.push_back("State.Immune");
		block.push_back("State.Dead");
		cue->set_activation_blocked_any_tags(block);
		CHECK(cue->get_activation_blocked_any_tags().size() == 2);
		CHECK(StringName(cue->get_activation_blocked_any_tags()[0]) == StringName("State.Immune"));
		CHECK(StringName(cue->get_activation_blocked_any_tags()[1]) == StringName("State.Dead"));

		// Var 2: Replace — swap to different blocker
		TypedArray<StringName> block2;
		block2.push_back("State.Ethereal");
		cue->set_activation_blocked_any_tags(block2);
		CHECK(cue->get_activation_blocked_any_tags().size() == 1);
		CHECK(StringName(cue->get_activation_blocked_any_tags()[0]) == StringName("State.Ethereal"));

		// Var 3: Empty — no OR block
		cue->set_activation_blocked_any_tags(TypedArray<StringName>());
		CHECK(cue->get_activation_blocked_any_tags().size() == 0);
	}

	SUBCASE("Activation Blocked All Tags (AND) - 3 Variations") {
		// Var 1: Round-trip — AND combo, verify content
		TypedArray<StringName> block;
		block.push_back("action.jumping");
		block.push_back("action.attacking");
		cue->set_activation_blocked_all_tags(block);
		CHECK(cue->get_activation_blocked_all_tags().size() == 2);
		CHECK(StringName(cue->get_activation_blocked_all_tags()[0]) == StringName("action.jumping"));
		CHECK(StringName(cue->get_activation_blocked_all_tags()[1]) == StringName("action.attacking"));

		// Var 2: Replace — different AND combo
		TypedArray<StringName> block2;
		block2.push_back("state.stunned");
		block2.push_back("state.silenced");
		cue->set_activation_blocked_all_tags(block2);
		CHECK(cue->get_activation_blocked_all_tags().size() == 2);
		CHECK(StringName(cue->get_activation_blocked_all_tags()[0]) == StringName("state.stunned"));

		// Var 3: Empty — no AND block
		cue->set_activation_blocked_all_tags(TypedArray<StringName>());
		CHECK(cue->get_activation_blocked_all_tags().size() == 0);
	}
}
