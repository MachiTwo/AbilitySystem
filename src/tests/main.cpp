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

int ability_system_run_tests(int p_mode) {
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
#include "src/resources/ability_system_ability.h"
#include "src/resources/ability_system_ability_container.h"
#include "src/resources/ability_system_attribute.h"
#include "src/resources/ability_system_attribute_set.h"
#include "src/resources/ability_system_cue.h"
#include "src/resources/ability_system_cue_animation.h"
#include "src/resources/ability_system_cue_audio.h"
#include "src/resources/ability_system_effect.h"
#include "src/scene/ability_system_component.h"
#else
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_ability_container.h"
#include "modules/ability_system/resources/ability_system_attribute.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"
#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/resources/ability_system_cue_animation.h"
#include "modules/ability_system/resources/ability_system_cue_audio.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/scene/ability_system_component.h"
#endif

#include "test_ability_system.h"
#include "test_ability_system_ability.h"
#include "test_ability_system_ability_container.h"
#include "test_ability_system_ability_spec.h"
#include "test_ability_system_attribute.h"
#include "test_ability_system_attribute_set.h"
#include "test_ability_system_component.h"
#include "test_ability_system_coverage_gap.h"
#include "test_ability_system_cue_animation.h"
#include "test_ability_system_cue_audio.h"
#include "test_ability_system_cue_spec.h"
#include "test_ability_system_effect.h"
#include "test_ability_system_effect_spec.h"
#include "test_ability_system_integration.h"
#include "test_ability_system_symmetry.h"
#include "test_ability_system_tag_spec.h"
#include "test_ability_system_task.h"
