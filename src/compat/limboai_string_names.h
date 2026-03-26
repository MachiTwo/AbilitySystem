/**************************************************************************/
/*  limboai_string_names.h                                                */
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

/**
 * compat/limboai_string_names.h
 * =============================================================================
 * Compatibility wrapper for LimboAI's internal StringNames.
 * Provides a unified way to access commonly used strings in LimboAI.
 * =============================================================================
 */

#ifndef COMPAT_LIMBOAI_STRING_NAMES_H
#define COMPAT_LIMBOAI_STRING_NAMES_H

#if defined(LIMBOAI_MODULE) || defined(LIMBOAI_GDEXTENSION)
#ifdef LIMBOAI_GDEXTENSION
#include "util/limbo_string_names.h"
#else
#include "modules/limboai/util/limbo_string_names.h"
#endif
#else

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include <godot_cpp/variant/string_name.hpp>
using namespace godot;
#else
#include "core/string/string_name.h"
#endif

class LimboStringNames {
	static LimboStringNames *singleton;

public:
	static LimboStringNames *get_singleton() { return singleton; }

	static void create();
	static void free();

	StringName _setup;
	StringName _enter;
	StringName _exit;
	StringName _update;
	StringName _tick;
	StringName _on_as_event_received;
	StringName agent;
	StringName blackboard;
	StringName blackboard_plan;
	StringName LimboAI;

	LimboStringNames();
};

#define LW_SN (LimboStringNames::get_singleton())

#endif // !LIMBOAI_MODULE

#endif // COMPAT_LIMBOAI_STRING_NAMES_H
