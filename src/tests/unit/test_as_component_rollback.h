/**************************************************************************/
/*  test_as_component_rollback.h                                          */
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

#ifndef TEST_AS_COMPONENT_ROLLBACK_H
#define TEST_AS_COMPONENT_ROLLBACK_H

// [RED STATE] — rollback_to_tick / capture_snapshot / current_tick are internal
// multiplayer/prediction infrastructure (protected). Public rollback API does not
// exist yet. These tests validate only the externally observable state mutations
// that a GDScript/C# user can perform and inspect.
// When a public rollback API is introduced (e.g. asc->take_snapshot() / asc->restore_snapshot()),
// the RED sub-cases below must be promoted to full GREEN tests.

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/resources/as_attribute.h"
#include "src/resources/as_attribute_set.h"
#include "src/scene/as_component.h"
#include "src/tests/doctest.h"
#else
#include "modules/ability_system/resources/as_attribute.h"
#include "modules/ability_system/resources/as_attribute_set.h"
#include "modules/ability_system/scene/as_component.h"
#include "modules/ability_system/tests/doctest.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

// Helper: builds a minimal ASComponent with a Health attribute.
static ASComponent *_make_asc_rollback(float p_hp = 100.0f) {
	ASComponent *asc = memnew(ASComponent);

	Ref<ASAttributeSet> aset;
	aset.instantiate();

	Ref<ASAttribute> hp;
	hp.instantiate();
	hp->set_attribute_name("Health");
	hp->set_base_value(p_hp);
	hp->set_min_value(0.0f);
	hp->set_max_value(p_hp);
	aset->add_attribute_definition(hp);

	asc->add_attribute_set(aset);
	return asc;
}

TEST_CASE("[ASComponent] Attribute Mutation (Public API — Rollback Preconditions)") {
	// NOTE: This test validates public state-mutation semantics that the rollback
	// system builds upon. The actual rollback (take_snapshot / restore_snapshot)
	// is a [RED STATE] — its public API does not yet exist.

	ASComponent *asc = _make_asc_rollback(100.0f);

	SUBCASE("set_attribute_base_value_by_tag — consecutive mutations - 3 Variations") {
		// Var 1: Initial value is as configured
		CHECK(asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(100.0f));

		// Var 2: Single mutation is applied and queryable
		asc->set_attribute_base_value_by_tag(StringName("Health"), 80.0f);
		CHECK(asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(80.0f));

		// Var 3: Second mutation overwrites the first
		asc->set_attribute_base_value_by_tag(StringName("Health"), 50.0f);
		CHECK(asc->get_attribute_base_value_by_tag(StringName("Health")) == doctest::Approx(50.0f));
	}

	SUBCASE("Tag mutation — add/remove observable state - 3 Variations") {
		StringName stun = StringName("State.Stun");

		// Var 1: Tag absent before add
		CHECK_FALSE(asc->has_tag(stun));

		// Var 2: Tag present after add
		asc->add_tag(stun);
		CHECK(asc->has_tag(stun));

		// Var 3: Tag absent after remove (manual state restore, mirrors what rollback would do)
		asc->remove_tag(stun);
		CHECK_FALSE(asc->has_tag(stun));
	}

	SUBCASE("Snapshot persistence via set_snapshot_state — 3 Variations") {
		// Var 1: No snapshot by default
		CHECK(asc->get_snapshot_state().is_null());

		// Var 2: Assign a snapshot resource and retrieve it
		Ref<ASStateSnapshot> snap;
		snap.instantiate();
		asc->set_snapshot_state(snap);
		CHECK(asc->get_snapshot_state().is_valid());

		// Var 3: Replacing with null clears it
		asc->set_snapshot_state(Ref<ASStateSnapshot>());
		CHECK(asc->get_snapshot_state().is_null());
	}

	memdelete(asc);
}

#endif // TEST_AS_COMPONENT_ROLLBACK_H
