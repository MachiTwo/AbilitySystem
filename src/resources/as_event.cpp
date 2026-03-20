/**************************************************************************/
/*  as_event.cpp                                                          */
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
#include "src/resources/as_event.h"
#else
#include "modules/ability_system/resources/as_event.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

void ASEvent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_event_tag", "tag"), &ASEvent::set_event_tag);
	ClassDB::bind_method(D_METHOD("get_event_tag"), &ASEvent::get_event_tag);

	ClassDB::bind_method(D_METHOD("set_instigator", "instigator"), &ASEvent::set_instigator);
	ClassDB::bind_method(D_METHOD("get_instigator"), &ASEvent::get_instigator);

	ClassDB::bind_method(D_METHOD("set_magnitude", "magnitude"), &ASEvent::set_magnitude);
	ClassDB::bind_method(D_METHOD("get_magnitude"), &ASEvent::get_magnitude);

	ClassDB::bind_method(D_METHOD("set_custom_payload", "payload"), &ASEvent::set_custom_payload);
	ClassDB::bind_method(D_METHOD("get_custom_payload"), &ASEvent::get_custom_payload);

	ClassDB::bind_method(D_METHOD("set_timestamp", "timestamp"), &ASEvent::set_timestamp);
	ClassDB::bind_method(D_METHOD("get_timestamp"), &ASEvent::get_timestamp);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "event_tag"), "set_event_tag", "get_event_tag");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "instigator", PROPERTY_HINT_RESOURCE_TYPE, "Node"), "set_instigator", "get_instigator");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "magnitude"), "set_magnitude", "get_magnitude");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "custom_payload"), "set_custom_payload", "get_custom_payload");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "timestamp"), "set_timestamp", "get_timestamp");
}

ASEvent::ASEvent() {
}

ASEvent::~ASEvent() {
}
