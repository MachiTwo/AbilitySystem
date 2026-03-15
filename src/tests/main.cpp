/**************************************************************************/
/*  main.cpp                                                              */
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

#ifdef ABILITY_SYSTEM_GDEXTENSION
#define DOCTEST_CONFIG_IMPLEMENT
#include "src/tests/doctest.h"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#else
#include "core/string/print_string.h"
#include "core/variant/variant.h"
#include "modules/ability_system/tests/doctest.h"
#endif

int as_run_tests(int p_mode) {
#ifdef ABILITY_SYSTEM_GDEXTENSION
	godot::UtilityFunctions::print("[ABILITY SYSTEM] Starting tests in mode: ", p_mode);
#else
	print_line(vformat("[ABILITY SYSTEM] Starting tests in mode: %d", p_mode));
#endif
	doctest::Context context;
	if (p_mode == 1) {
		const char *argv[] = { "godot", "--test-case-exclude=*Integration*", "--success" };
		context.applyCommandLine(3, argv);
	} else if (p_mode == 2) {
		const char *argv[] = { "godot", "--test-case=*Integration*", "--success" };
		context.applyCommandLine(3, argv);
	} else {
		const char *argv[] = { "godot", "--success" };
		context.applyCommandLine(2, argv);
	}
	return context.run();
}

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/core/ability_system.h"
#include "src/resources/as_ability.h"
#include "src/resources/as_attribute.h"
#include "src/resources/as_attribute_set.h"
#include "src/resources/as_container.h"
#include "src/resources/as_cue.h"
#include "src/resources/as_cue_animation.h"
#include "src/resources/as_cue_audio.h"
#include "src/resources/as_effect.h"
#include "src/scene/as_component.h"
#else
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/resources/as_ability.h"
#include "modules/ability_system/resources/as_attribute.h"
#include "modules/ability_system/resources/as_attribute_set.h"
#include "modules/ability_system/resources/as_container.h"
#include "modules/ability_system/resources/as_cue.h"
#include "modules/ability_system/resources/as_cue_animation.h"
#include "modules/ability_system/resources/as_cue_audio.h"
#include "modules/ability_system/resources/as_effect.h"
#include "modules/ability_system/scene/as_component.h"
#endif

#include "test_ability_system.h"
#include "test_as_ability.h"
#include "test_as_ability_spec.h"
#include "test_as_advanced_logic.h"
#include "test_as_attribute_set.h"
#include "test_as_cancel_tags.h"
#include "test_as_component.h"
#include "test_as_container.h"
#include "test_as_coverage_gap.h"
#include "test_as_cue_animation.h"
#include "test_as_cue_audio.h"
#include "test_as_cue_spec.h"
#include "test_as_delivery.h"
#include "test_as_effect.h"
#include "test_as_effect_spec.h"
#include "test_as_integration.h"
#include "test_as_mega_integration.h"
#include "test_as_multiple_effects.h"
#include "test_as_package.h"
#include "test_as_stun_refactor.h"
#include "test_as_symmetry.h"
#include "test_as_tag_spec.h"
#include "test_as_triggers.h"
