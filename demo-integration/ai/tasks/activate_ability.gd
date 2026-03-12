@tool
extends BTAction
## Actives a specific AbilitySystem ability on the agent.
## Returns SUCCESS if the ability was successfully activated, FAILURE otherwise.

## The tag of the ability to activate.
@export var ability_tag: StringName = &""

## If true, the task will return RUNNING until the ability ends.
## Note: Only use this for abilities with a defined duration or that end automatically.
## Do NOT use for state abilities like 'idle' or 'walk'.
@export var wait_until_ended: bool = false

var _activated_this_session: bool = false

func _generate_name() -> String:
	return "ActivateAbility " + String(ability_tag)

func _enter() -> void:
	_activated_this_session = false

func _tick(_delta: float) -> Status:
	var asc: ASComponent = agent.get_node_or_null("ASComponent")
	if not is_instance_valid(asc):
		return FAILURE

	if ability_tag == &"":
		return FAILURE

	# PROTEÇÃO: Se for uma tag de estado, não esperamos o fim.
	var should_wait = wait_until_ended
	if String(ability_tag).begins_with("state."):
		should_wait = false

	# 1. Check if it is already active
	var active_spec = null
	var active_specs = asc.get_active_abilities_debug()
	for s in active_specs:
		if is_instance_valid(s) and s.get_ability() and s.get_ability().get_ability_tag() == ability_tag:
			active_spec = s
			break
	
	if active_spec:
		if should_wait:
			return RUNNING
		return SUCCESS

	# 2. If we were waiting and it's no longer active, we are done
	if _activated_this_session and should_wait:
		return SUCCESS

	# 3. Try to activate
	if asc.can_activate_ability_by_tag(ability_tag):
		var success = asc.try_activate_ability_by_tag(ability_tag)
		if success:
			_activated_this_session = true
			if should_wait:
				return RUNNING
			return SUCCESS
	
	# If we can't activate and it's not active, return failure
	return FAILURE
