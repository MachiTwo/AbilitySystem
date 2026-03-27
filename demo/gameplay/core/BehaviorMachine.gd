class_name BehaviorMachine
extends Node

## Behavior Engineering Framework - Core Machine
##
## Base class for Context-Aware State Machines.
## Handles Context management, State Transitions, and the "Best Match" Scoring Algorithm.

signal state_changed(new_res: Resource)
signal context_changed(category: String, value: int)

# State
var current_state_res: Resource
var time_in_state: float = 0.0

# Context (The "Memory" of the machine)
var context: Dictionary = {}
var _cooldowns: Dictionary = {}
var _context_cooldowns: Dictionary = {}

# Debug
@export var debug_enabled: bool = false

func _process(delta: float) -> void:
	if current_state_res:
		time_in_state += delta

# --- Context API ---

func set_context(category: String, value: int) -> void:
	# Optimization: If value is same, ignore
	if context.get(category) == value: return
	
	context[category] = value
	if debug_enabled: print("[%s] Context Changed: %s -> %d" % [name, category, value])
	emit_signal("context_changed", category, value)
	_on_context_updated(category, value)

# Virtual: Children implement this to react to context changes
func _on_context_updated(_category: String, _value: int) -> void:
	pass

# --- State API ---

func change_state(new_res: Resource, preserve_time: bool = false) -> void:
	if current_state_res == new_res and not preserve_time: return
	
	# Apply cooldowns on entry
	if new_res:
		apply_cooldown(new_res)
	
	current_state_res = new_res
	if not preserve_time:
		time_in_state = 0.0
		
	emit_signal("state_changed", new_res)
	
	if debug_enabled:
		if new_res:
			print("[%s] State: %s" % [name, new_res.get("name")])
		else:
			print("[%s] State: Stopped" % name)

## Applies cooldowns to the state and associated context.
func apply_cooldown(res: Resource) -> void:
	var time = Time.get_ticks_msec() / 1000.0
	
	# State Cooldown
	if "cooldown" in res and res.cooldown > 0:
		_cooldowns[res] = time
		
	# Context Cooldown
	if "context_cooldown_filter" in res and res.context_cooldown_filter != BehaviorStates.ContextFilter.NONE:
		if res.context_cooldown_time > 0:
			var key = _get_context_key_from_filter(res, res.context_cooldown_filter)
			if key != "":
				_context_cooldowns[key] = time + res.context_cooldown_time
				if debug_enabled: print("[%s] Context Cooldown Applied: %s for %.1fs" % [name, key, res.context_cooldown_time])

func _get_context_key_from_filter(res: Resource, filter: int) -> String:
	var reqs = res.get("entry_requirements")
	if not reqs: return ""

	var category = ""
	var val = 0
	
	match filter:
		BehaviorStates.ContextFilter.MOTION:
			category = "Motion"
			val = reqs.get("motion", 0)
		BehaviorStates.ContextFilter.JUMP:
			category = "Jump"
			val = reqs.get("jump", 0)
		BehaviorStates.ContextFilter.ATTACK:
			category = "Attack"
			val = reqs.get("attack", 0)
		BehaviorStates.ContextFilter.PHYSICS:
			category = "Physics"
			val = reqs.get("physics", 0)
		BehaviorStates.ContextFilter.STATUS:
			category = "Status"
			val = reqs.get("status", 0)
		BehaviorStates.ContextFilter.WEAPON:
			category = "Weapon"
			val = reqs.get("weapon", 0)
			
	if category != "":
		return "%s:%d" % [category, val]
	return ""

# --- Scoring Algorithm (The Core) ---

func find_best_match(candidates: Array[Resource]) -> Resource:
	var best_candidate: Resource = null
	var best_score: int = -9999
	
	for res in candidates:
		if not res: continue
		
		var score = 0
		var possible = true
		
		# 1.0 Previous States Filter (Chain/Combo) - PRIORITY
		var is_chain_valid = false
		var has_chain_req = false
		
		if "previous_states" in res and not res.previous_states.is_empty():
			has_chain_req = true
			if current_state_res:
				if current_state_res in res.previous_states:
					is_chain_valid = true
			
			if not is_chain_valid:
				possible = false
				continue
			else:
				if debug_enabled: print("  - %s: Chain Valid! Bypassing Cooldowns." % res.name)
				score += 2

		# Iterate over all categories present in current context
		for category in context.keys():
			var ctx_val = context[category]
			
			# Map to resource property (e.g. req_weapon)
			var req_key = category.to_lower()
			
			# Read Requirements Dictionary
			var req_dict = res.get("entry_requirements")
			
			# Maintenance Logic
			if res == current_state_res:
				var maint = res.get("maintenance_requirements")
				if maint and not maint.is_empty():
					req_dict = maint
			
			var req_val = null
			if req_dict and req_dict.has(req_key):
				req_val = req_dict[req_key]
			
			if req_val != null:
				# 1. Hard Filter
				var is_match = BehaviorStates.check_match(category, req_val, ctx_val)
				
				if not is_match:
					possible = false
					# if debug_enabled: print("  - %s: Failed Match on %s" % [res.name, category])
					break
					
				# 1.0.1 Context Cooldown Filter
				if not is_chain_valid:
					var ctx_key = "%s:%d" % [category, req_val]
					if _context_cooldowns.has(ctx_key):
						var end_time = _context_cooldowns[ctx_key]
						var current_time = Time.get_ticks_msec() / 1000.0
						if current_time < end_time:
							possible = false
							if debug_enabled: print("  - %s: Context Cooldown on %s" % [res.name, ctx_key])
							break
				
				# 1.1 Scoring
				if req_val != 0 and req_val == ctx_val:
					score += 10
				elif req_val > 0:
					score += 5
					
		if not possible: continue
		
		# 1.4 Health Requirements
		var req_min_hp = res.get("req_min_hp")
		var req_max_hp = res.get("req_max_hp")
		var current_hp = get_resource_value(BehaviorStates.CostType.HEALTH)
		
		if req_min_hp and current_hp < req_min_hp:
			possible = false
			continue
		if req_max_hp and current_hp > req_max_hp:
			possible = false
			continue

		# 1.5 Resource Cost Check
		var cost_type = res.get("cost_type")
		var cost_amount = res.get("cost_amount")
		
		if cost_type and cost_amount and cost_amount > 0:
			var available = get_resource_value(cost_type)
			if available < cost_amount:
				var rule = res.get("on_insufficient_resource")
				if rule == null or rule == BehaviorStates.LowResourceRule.IGNORE_COMMAND:
					possible = false
					if debug_enabled: print("  - %s: Insufficient Resource" % res.name)
					# continue? No, logic in PlayerMachine was just setting possible=false
		
		if not possible: continue

		# 1.6 Cooldown Check
		if "cooldown" in res and res.cooldown > 0:
			var last_time = _cooldowns.get(res, -1.0)
			var current_time = Time.get_ticks_msec() / 1000.0
			if last_time > 0 and current_time < last_time + res.cooldown:
				possible = false
				if debug_enabled: print("  - %s: Cooldown" % res.name)
				continue
		
		# Manual Priority Override
		var priority = res.get("priority_override")
		if priority: score += priority
		
		if score > best_score:
			best_score = score
			best_candidate = res
			
	return best_candidate

## Virtual: Returns current value for a resource type (Health, Stamina, etc).
func get_resource_value(_type: int) -> float:
	return 9999.0
