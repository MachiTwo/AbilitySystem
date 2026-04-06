/**************************************************************************/
/*  limboai_bt.h                                                          */
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
 * compat/limboai_bt.h
 * =============================================================================
 * Compatibility wrapper for LimboAI Behavior Tree core classes
 *
 * When LIMBOAI_MODULE is defined, includes the real LimboAI headers.
 * When not available, provides stub implementations for compilation.
 * =============================================================================
 */

#ifndef COMPAT_LIMBOAI_BT_H
#define COMPAT_LIMBOAI_BT_H

#if defined(LIMBOAI_MODULE) || defined(LIMBOAI_GDEXTENSION)
// Real LimboAI available — use actual headers
#ifdef LIMBOAI_GDEXTENSION
#include "src/limboai/bt/behavior_tree.h"
#include "src/limboai/bt/tasks/bt_action.h"
#include "src/limboai/bt/tasks/bt_condition.h"
#include "src/limboai/bt/tasks/bt_task.h"
#else
#include "modules/ability_system/limboai/bt/behavior_tree.h"
#include "modules/ability_system/limboai/bt/tasks/bt_action.h"
#include "modules/ability_system/limboai/bt/tasks/bt_condition.h"
#include "modules/ability_system/limboai/bt/tasks/bt_task.h"
#endif
#else
// Stub implementations follow

#include "../scene/as_component.h"
#include "limboai_blackboard.h"

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/variant.hpp>
using namespace godot;

#else
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/templates/vector.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "scene/main/node.h"
#endif

// Forward declarations
class BTTask;
class BTAction;
class BTCondition;
class BTComposite;
class BTDecorator;
class BTLeaf;
class BehaviorTree;
class BTInstance;
class Blackboard;

// LimboAI Task Macros
#ifndef TASK_CATEGORY
#define TASK_CATEGORY(m_category)
#endif

/**
 * BT namespace for Behavior Tree status codes
 */
namespace BT {
enum Status {
	FRESH = 0, // Just created, not yet executed
	RUNNING = 1, // Currently executing
	SUCCESS = 2, // Completed successfully
	FAILURE = 3 // Failed to complete
};
}

VARIANT_ENUM_CAST(BT::Status)

/**
 * BTTask - Base class for all Behavior Tree tasks
 *
 * Compatibility wrapper for LimboAI's BTTask.
 * Provides the fundamental interface for all BT nodes.
 */
class BTTask : public RefCounted {
	GDCLASS(BTTask, RefCounted)

protected:
	static void _bind_methods();

public:
	virtual ~BTTask() = default;

	// Public BT lifecycle (LimboAI API)
	virtual void enter() { _enter(); }
	virtual void exit() { _exit(); }
	virtual BT::Status tick(double p_delta) { return _tick(p_delta); }

	// Core BT lifecycle overrides
	virtual void _setup() {}
	virtual void _enter() {}
	virtual void _exit() {}
	virtual BT::Status _tick(double p_delta) = 0;

	// Agent management
	virtual void set_agent(Node *p_agent) { agent = p_agent; }
	virtual Node *get_agent() const { return agent; }

	// Blackboard management
	virtual void set_blackboard(const Ref<Blackboard> &p_blackboard) { blackboard = p_blackboard; }
	virtual Ref<Blackboard> get_blackboard() const { return blackboard; }

	// Child management (for composite nodes)
	virtual void add_child(const Ref<BTTask> &p_child) {}
	virtual void remove_child(const Ref<BTTask> &p_child) {}
	virtual TypedArray<BTTask> get_children() const { return TypedArray<BTTask>(); }
	virtual int get_child_count() const { return 0; }
	virtual Ref<BTTask> get_child(int p_idx) const { return Ref<BTTask>(); }

	virtual void set_status(int p_status) {}
	_FORCE_INLINE_ int get_status() const { return 0; }
	virtual void set_elapsed_time(double p_elapsed) {}
	_FORCE_INLINE_ double get_elapsed_time() const { return 0.0; }

	// Utility methods
	virtual String get_task_name() const { return "BTTask"; }
	virtual String get_category() const { return ""; }

protected:
	Node *agent = nullptr;
	Ref<Blackboard> blackboard;
};

/**
 * BTSequence - Executes children in order until one fails or all succeed
 */
class BTSequence : public BTComposite {
	GDCLASS(BTSequence, BTComposite)
public:
	virtual BT::Status _tick(double p_delta) override {
		for (int i = 0; i < get_child_count(); i++) {
			BT::Status status = get_child(i)->tick(p_delta);
			if (status != BT::SUCCESS)
				return status;
		}
		return BT::SUCCESS;
	}
};

/**
 * BTSelector - Executes children in order until one succeeds
 */
class BTSelector : public BTComposite {
	GDCLASS(BTSelector, BTComposite)
public:
	virtual BT::Status _tick(double p_delta) override {
		for (int i = 0; i < get_child_count(); i++) {
			BT::Status status = get_child(i)->tick(p_delta);
			if (status == BT::SUCCESS)
				return BT::SUCCESS;
		}
		return BT::FAILURE;
	}
};

/**
 * BTLeaf - Base class for tasks with no children
 */
class BTLeaf : public BTTask {
	GDCLASS(BTLeaf, BTTask)
protected:
	static void _bind_methods() {}

public:
	virtual BT::Status _tick(double p_delta) override { return BT::SUCCESS; }
};

/**
 * BTComposite - Base class for tasks with multiple children
 */
class BTComposite : public BTTask {
	GDCLASS(BTComposite, BTTask)
private:
	Vector<Ref<BTTask>> children;

protected:
	static void _bind_methods();

public:
	virtual BT::Status _tick(double p_delta) override { return BT::SUCCESS; }
	virtual void add_child(const Ref<BTTask> &p_child) override { children.push_back(p_child); }
	virtual void remove_child(const Ref<BTTask> &p_child) override { children.erase(p_child); }
	virtual TypedArray<BTTask> get_children() const override;
	virtual int get_child_count() const override { return children.size(); }
	virtual Ref<BTTask> get_child(int p_idx) const override { return children[p_idx]; }
};

/**
 * BTDecorator - Base class for tasks with exactly one child
 */
class BTDecorator : public BTTask {
	GDCLASS(BTDecorator, BTTask)
private:
	Ref<BTTask> child;

protected:
	static void _bind_methods();

public:
	virtual BT::Status _tick(double p_delta) override { return BT::SUCCESS; }
	virtual void add_child(const Ref<BTTask> &p_child) override { child = p_child; }
	virtual void remove_child(const Ref<BTTask> &p_child) override {
		if (child == p_child)
			child = Ref<BTTask>();
	}
	virtual TypedArray<BTTask> get_children() const override;
	virtual int get_child_count() const override { return child.is_valid() ? 1 : 0; }
	virtual Ref<BTTask> get_child(int p_idx) const override { return child; }
};

/**
 * BTAction - Base class for Behavior Tree action nodes
 *
 * Compatibility wrapper for LimboAI's BTAction.
 * Actions are leaf nodes that perform specific behaviors.
 */
class BTAction : public BTLeaf {
	GDCLASS(BTAction, BTLeaf)

protected:
	static void _bind_methods();

public:
	virtual ~BTAction() = default;
};

/**
 * BTCondition - Base class for Behavior Tree condition nodes
 *
 * Compatibility wrapper for LimboAI's BTCondition.
 * Conditions are leaf nodes that test specific states.
 */
class BTCondition : public BTLeaf {
	GDCLASS(BTCondition, BTLeaf)

protected:
	static void _bind_methods();

public:
	virtual ~BTCondition() = default;
};

/**
 * BehaviorTree - Resource containing BT structure
 *
 * Compatibility wrapper for LimboAI's BehaviorTree.
 * Defines the tree structure and can be instantiated for execution.
 */
class BehaviorTree : public Resource {
	GDCLASS(BehaviorTree, Resource)

private:
	Ref<BTTask> root_task;
	String description;

protected:
	static void _bind_methods();

public:
	void set_root_task(const Ref<BTTask> &p_task) { root_task = p_task; }
	Ref<BTTask> get_root_task() const { return root_task; }

	void set_description(const String &p_desc) { description = p_desc; }
	String get_description() const { return description; }

	// Create an instance for execution
	Ref<BTInstance> instantiate(Node *p_agent);

	BehaviorTree() = default;
	~BehaviorTree() = default;
};

/**
 * BTInstance - Runtime instance of a BehaviorTree
 *
 * Compatibility wrapper for LimboAI's BTInstance.
 * Manages execution state for a specific agent.
 */
class BTInstance : public RefCounted {
	GDCLASS(BTInstance, RefCounted)

private:
	Ref<BehaviorTree> behavior_tree;
	Node *agent = nullptr;
	Ref<BTTask> current_task;
	BT::Status last_status = BT::FRESH;

protected:
	static void _bind_methods();

public:
	void initialize(Node *p_agent, const Ref<BehaviorTree> &p_tree);

	BT::Status tick(double p_delta);
	void reset();

	Node *get_agent() const { return agent; }
	Ref<BehaviorTree> get_behavior_tree() const { return behavior_tree; }
	BT::Status get_last_status() const { return last_status; }

	Dictionary capture_state() const { return Dictionary(); }
	void restore_state(const Dictionary &p_dict) {}

	BTInstance() = default;
	~BTInstance() = default;
};

#endif // !LIMBOAI_MODULE

#endif // COMPAT_LIMBOAI_BT_H
