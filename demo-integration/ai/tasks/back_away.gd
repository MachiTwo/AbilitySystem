#*
#* back_away.gd
#* =============================================================================
#* Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*
@tool
extends BTAction
## Moves the agent in the opposite direction of its current facing. [br]
## Returns [code]RUNNING[/code] always.

## Blackboard variable that stores desired speed.
@export var speed_var: StringName = &"speed"

## How much can we deviate from the "away" direction (in radians).
@export var max_angle_deviation: float = 0.7

var _dir: Vector2
var _desired_velocity: Vector2


# Called each time this task is entered.
func _enter() -> void:
	# Determine "away" direction and desired velocity
	var facing = agent.get_facing()
	if facing == 0.0: facing = 1.0 # Default to right if not facing anywhere
	_dir = Vector2.LEFT * facing
	var speed: float = blackboard.get_var(speed_var, 200.0)
	var rand_angle = randf_range(-max_angle_deviation, max_angle_deviation)
	_desired_velocity = _dir.rotated(rand_angle) * speed


# Called each time this task is ticked (aka executed).
func _tick(_delta: float) -> Status:
	if not is_instance_valid(agent): return FAILURE
	agent.move(_desired_velocity)
	agent.face_dir(-signf(_dir.x))
	return RUNNING
