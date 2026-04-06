/**************************************************************************/
/*  limboai_utility.h                                                     */
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
 * compat/limboai_utility.h
 * =============================================================================
 * Compatibility wrapper for LimboAI utility functions.
 * Provides helper methods for class discovery and editor integration.
 * =============================================================================
 */

#ifndef COMPAT_LIMBOAI_UTILITY_H
#define COMPAT_LIMBOAI_UTILITY_H

#if defined(LIMBOAI_MODULE) || defined(LIMBOAI_GDEXTENSION)
#ifdef LIMBOAI_GDEXTENSION
#include "util/limbo_utility.h"
#else
#include "modules/ability_system/limboai/util/limbo_utility.h"
#endif
#else

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>
using namespace godot;
#else
#include "core/object/object.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#endif

class LimboUtility : public Object {
	GDCLASS(LimboUtility, Object)

	static LimboUtility *singleton;

protected:
	static void _bind_methods();

public:
	static LimboUtility *get_singleton() { return singleton; }

	/**
	 * Returns a list of global classes that inherit from a specific base.
	 * Used for task discovery in the editor.
	 */
	virtual Array get_global_class_list(const String &p_base);

	LimboUtility();
	~LimboUtility();
};

#endif // !LIMBOAI_MODULE

#endif // COMPAT_LIMBOAI_UTILITY_H
