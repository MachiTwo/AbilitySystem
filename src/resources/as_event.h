/**************************************************************************/
/*  as_event.h                                                            */
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
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource.hpp>
#else
#include "core/io/resource.h"
#include "scene/main/node.h"
#endif

namespace godot {
class ASEvent : public Resource {
	GDCLASS(ASEvent, Resource);

private:
	StringName event_tag;
	Node *instigator = nullptr;
	float magnitude = 0.0f;
	Dictionary custom_payload;
	double timestamp = 0.0;

protected:
	static void _bind_methods();

public:
	void set_event_tag(const StringName &p_tag) { event_tag = p_tag; }
	StringName get_event_tag() const { return event_tag; }

	void set_instigator(Node *p_node) { instigator = p_node; }
	Node *get_instigator() const { return instigator; }

	void set_magnitude(float p_mag) { magnitude = p_mag; }
	float get_magnitude() const { return magnitude; }

	void set_custom_payload(const Dictionary &p_payload) { custom_payload = p_payload; }
	Dictionary get_custom_payload() const { return custom_payload; }

	void set_timestamp(double p_time) { timestamp = p_time; }
	double get_timestamp() const { return timestamp; }

	ASEvent();
	~ASEvent();
};
} // namespace godot
