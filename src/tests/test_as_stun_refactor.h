/**************************************************************************/
/*  test_as_stun_refactor.h                                               */
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

#ifndef TEST_AS_STUN_REFACTOR_H
#define TEST_AS_STUN_REFACTOR_H

#include "doctest.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/core/as_effect_spec.h"
#include "src/resources/as_effect.h"
#include "src/scene/as_component.h"
#else
#include "modules/ability_system/core/as_effect_spec.h"
#include "modules/ability_system/resources/as_effect.h"
#include "modules/ability_system/scene/as_component.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

TEST_CASE("[AbilitySystem] Stun Refactor: Stun as Effect") {
	ASComponent *asc = memnew(ASComponent);

	// 1. Create Stun Effect
	Ref<ASEffect> stun_effect;
	stun_effect.instantiate();
	stun_effect->set_effect_name("Stun Effect");
	stun_effect->set_duration_policy(ASEffect::POLICY_DURATION);
	stun_effect->set_duration_magnitude(1.0f); // 1 second stun

	TypedArray<StringName> granted_tags;
	granted_tags.push_back("state.stunned");
	stun_effect->set_granted_tags(granted_tags);

	SUBCASE("Applying stun effect grants state.stunned tag") {
		CHECK(asc->has_tag("state.stunned") == false);

		asc->apply_effect_by_resource(stun_effect);

		CHECK(asc->has_tag("state.stunned") == true);
	}

	SUBCASE("Stun tag is removed after effect duration") {
		asc->apply_effect_by_resource(stun_effect);
		CHECK(asc->has_tag("state.stunned") == true);

		// Process 1.1s (more than the 1.0s duration)
		asc->tick(1.1f);

		CHECK(asc->has_tag("state.stunned") == false);
	}

	SUBCASE("Stacking stun effects (Override policy)") {
		stun_effect->set_stacking_policy(ASEffect::STACK_OVERRIDE);

		asc->apply_effect_by_resource(stun_effect);
		asc->tick(0.5f);

		// Re-apply stun, should reset timer to 1.0s
		asc->apply_effect_by_resource(stun_effect);

		asc->tick(0.6f);
		// Total time 1.1s, but second application was at 0.5s, so it should still be active (until 1.5s)
		CHECK(asc->has_tag("state.stunned") == true);

		asc->tick(0.5f); // Total time 1.6s
		CHECK(asc->has_tag("state.stunned") == false);
	}

	memdelete(asc);
}

#endif // TEST_AS_STUN_REFACTOR_H
