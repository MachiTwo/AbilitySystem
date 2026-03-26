/**************************************************************************/
/*  limboai_utility.cpp                                                   */
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

#include "limboai_utility.h"

#if !defined(LIMBOAI_MODULE) && !defined(LIMBOAI_GDEXTENSION)

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#else
#include "core/config/project_settings.h"
#include "core/variant/dictionary.h"
#endif

LimboUtility *LimboUtility::singleton = nullptr;

void LimboUtility::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_global_class_list", "base"), &LimboUtility::get_global_class_list);
}

Array LimboUtility::get_global_class_list(const String &p_base) {
	Array result;
	Array classes = ProjectSettings::get_singleton()->get_global_class_list();
	for (int i = 0; i < classes.size(); i++) {
		Dictionary d = classes[i];
		if (d.has("base") && String(d["base"]) == p_base) {
			result.push_back(d);
		}
	}
	return result;
}

LimboUtility::LimboUtility() {
	if (singleton == nullptr) {
		singleton = this;
	}
}

LimboUtility::~LimboUtility() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

#endif // !LIMBOAI_MODULE && !LIMBOAI_GDEXTENSION
