class_name Player
extends CharacterBody2D

signal died

@export var asc: ASComponent
@export var hotbar: Hotbar

var is_stunned: bool = false

# Timers internos
var _coyote_timer: float = 0.0
var _jump_buffer_timer: float = 0.0
var _dash_coyote_timer: float = 0.0
var _hurt_timer: float = 0.0
var _stabilization_timer: float = 0.2

# Combat timers
var _attack_timer: float = 0.0
var _is_attacking: bool = false

# Logic State
var facing_direction: float = 1.0:
	set(value):
		if value != 0:
			facing_direction = value
			if scale.x != 0:
				scale.x = abs(scale.x) * sign(value)

# Referências internas
var _sprite: Node
var _anim_player: AnimationPlayer
var _hitbox: Area2D

# Estado atual rastreado
var _current_state_tag: StringName = &""

# Dash physics params
const DASH_SPEED_MULT := 3.0
const DASH_ACCEL := 5000.0
const DASH_DURATION := 0.5
const HYPERDASH_SPEED_MULT := 3.5
const HYPERDASH_DURATION := 0.6

# Attack durations
const COMBO_DURATION := 0.4
const HEAVY_DURATION := 0.6
const DASH_ATTACK_DURATION := 0.5

# --- Attribute Accessors (lê do ASAttributeSet via ASComponent) ---

## Retorna o valor atual de um atributo AS.
func _attr(attr_name: StringName) -> float:
	if asc:
		return asc.get_attribute_value_by_tag(attr_name)
	return 0.0

## Define o valor base de um atributo AS.
func _set_attr(attr_name: StringName, value: float) -> void:
	if asc:
		asc.set_attribute_base_value_by_tag(attr_name, value)

## Atalhos para atributos frequentes.
var current_health: float:
	get: return _attr(&"health")
	set(v): _set_attr(&"health", v)

var current_stamina: float:
	get: return _attr(&"stamina")
	set(v): _set_attr(&"stamina", v)

var max_health: float:
	get: return 100.0

var max_stamina: float:
	get: return 100.0

var max_speed: float:
	get: return _attr(&"max_speed")

var default_acceleration: float:
	get: return _attr(&"acceleration")

var default_friction: float:
	get: return _attr(&"friction")

var default_air_resistance: float:
	get: return _attr(&"air_resistance")

var jump_force: float:
	get: return _attr(&"jump_force")

var gravity_scale: float:
	get: return _attr(&"gravity_scale")

var stamina_regen_rate: float:
	get: return _attr(&"stamina_regen")

var coyote_time: float:
	get: return _attr(&"coyote_time")

var jump_buffer_time: float:
	get: return _attr(&"jump_buffer_time")

func _ready() -> void:
	if not asc:
		asc = find_child("ASComponent") as ASComponent

	# Encontra nós visuais
	_sprite = find_child("ColorRect")
	if not _sprite: _sprite = find_child("Sprite2D")

	_anim_player = find_child("AnimationPlayer")
	_hitbox = find_child("Hitbox")

	# Conecta sinais do ASComponent
	if asc:
		if asc.container:
			asc.apply_container(asc.container)
		
		asc.tag_changed.connect(_on_tag_changed)
		if _anim_player:
			asc.set_animation_player(_anim_player)
		if _sprite:
			asc.register_node(&"ColorRect", _sprite)
		if _hitbox:
			asc.register_node(&"Hitbox", _hitbox)

	# Inicializa Hotbar
	if hotbar:
		if not hotbar.is_connected("selection_changed", _on_hotbar_selection_changed):
			hotbar.connect("selection_changed", _on_hotbar_selection_changed)
		_on_hotbar_selection_changed(hotbar.get_current_item())

	call_deferred("_activate_initial_state")

func _on_hotbar_selection_changed(item: Resource) -> void:
	if not asc: return
	
	# 1. Limpa todas as tags de arma atuais (Lowercase weapon.*)
	var current_tags = asc.get_tags()
	for tag in current_tags:
		if String(tag).begins_with("weapon."):
			asc.remove_tag(tag)
	
	# 2. Concede a nova tag baseada no item
	if item and "weapon_tag" in item and not item.weapon_tag.is_empty():
		var w_tag = String(item.weapon_tag).to_lower()
		asc.add_tag(StringName(w_tag))
		print("[Player] Weapon Swapped: ", w_tag)
	
	# 3. Unlock de habilidade única (opcional)
	if item and "ability" in item and item.ability is ASAbility:
		asc.unlock_ability_by_resource(item.ability)

func _activate_initial_state() -> void:
	if asc:
		asc.try_activate_ability_by_tag(&"state.idle")

func _physics_process(delta: float) -> void:
	if Engine.is_editor_hint(): return

	# Estabilização de Física (Frame 0 Fix)
	if _stabilization_timer > 0:
		_stabilization_timer -= delta
		move_and_slide()
		velocity = Vector2.ZERO
		return

	# Atualiza Timers
	var ct = coyote_time if coyote_time > 0 else 0.15
	if is_on_floor():
		_coyote_timer = ct
	else:
		_coyote_timer -= delta

	if _dash_coyote_timer > 0:
		_dash_coyote_timer -= delta

	if _hurt_timer > 0:
		_hurt_timer -= delta

	var jbt = jump_buffer_time if jump_buffer_time > 0 else 0.1
	if Input.is_action_just_pressed("jump"):
		_jump_buffer_timer = jbt
	else:
		_jump_buffer_timer -= delta

	# Attack timer
	if _is_attacking:
		_attack_timer -= delta
		if _attack_timer <= 0:
			_finish_attack()

	_handle_gravity(delta)
	_handle_input()
	_handle_movement(delta)
	move_and_slide()
	_update_state_from_physics()

func _handle_gravity(delta: float) -> void:
	if _current_state_tag == &"state.dash" or _current_state_tag == &"state.hyperdash":
		velocity.y = 0
		return

	if not is_on_floor():
		var gs = gravity_scale if gravity_scale > 0 else 1.0
		var g = 980.0 * gs
		velocity.y += g * delta

func _handle_input() -> void:
	if current_health <= 0: return

	# Dash Input
	if Input.is_action_just_pressed("dash") and current_stamina >= 10:
		if _try_activate(&"state.dash"):
			asc.apply_effect_by_tag(&"effect.dash_cost")
			_start_dash()
			return

	# Jump Input
	if _jump_buffer_timer > 0 and _coyote_timer > 0 and not _is_attacking:
		if _try_activate(&"state.jump"):
			var jf = jump_force if jump_force > 0 else 500.0
			velocity.y = -jf
			_jump_buffer_timer = 0
			_coyote_timer = 0
			return

func _unhandled_input(event: InputEvent) -> void:
	if not asc: return
	if current_health <= 0: return

	# Weapon Switching via Hotbar
	if event.is_action_pressed("hotbar_1"):
		hotbar.select_slot(0)
	elif event.is_action_pressed("hotbar_2"):
		hotbar.select_slot(1)
	elif event.is_action_pressed("hotbar_3"):
		hotbar.select_slot(2)

	# Attack Input
	if event.is_action_pressed("attack_light"):
		_try_start_attack(&"attack.combo1", COMBO_DURATION, 10)
	elif event.is_action_pressed("attack_heavy"):
		_try_start_attack(&"attack.heavy", HEAVY_DURATION, 15)

func _try_start_attack(tag: StringName, dur: float, dmg: int) -> void:
	if _try_activate(tag):
		_is_attacking = true
		_attack_timer = dur
		if _hitbox:
			_hitbox.position = Vector2(30 * facing_direction, 0)

func _finish_attack() -> void:
	_is_attacking = false
	_attack_timer = 0
	# Cancela qualquer habilidade de ataque ativa
	for tag in [&"attack.combo1", &"attack.combo2", &"attack.combo3", &"attack.heavy", &"attack.dash_attack"]:
		if asc.is_ability_active(tag):
			asc.cancel_ability_by_tag(tag)
	_update_state_from_physics()

func _start_dash() -> void:
	_dash_coyote_timer = DASH_DURATION
	var dash_dir = facing_direction
	var input_dir = Input.get_axis("ui_left", "ui_right")
	if input_dir: dash_dir = sign(input_dir)
	facing_direction = dash_dir

func _handle_movement(delta: float) -> void:
	var spd = max_speed if max_speed > 0 else 230.0
	var accel = default_acceleration if default_acceleration > 0 else 1200.0
	var fric = default_friction if default_friction > 0 else 1000.0
	var air_res = default_air_resistance if default_air_resistance > 0 else 200.0
	var can_control = true

	# Dash: locked movement
	if _current_state_tag == &"state.dash":
		can_control = false
		var target_speed = spd * DASH_SPEED_MULT * facing_direction
		velocity.x = move_toward(velocity.x, target_speed, DASH_ACCEL * delta)
		if is_on_wall():
			asc.cancel_ability_by_tag(&"state.dash")
			velocity.x = 0
			_dash_coyote_timer = 0
		return

	# HyperDash
	if _current_state_tag == &"state.hyperdash":
		can_control = false
		var target_speed = spd * HYPERDASH_SPEED_MULT * facing_direction
		velocity.x = move_toward(velocity.x, target_speed, DASH_ACCEL * delta)
		return

	# Attacking: preserve momentum
	if _is_attacking:
		can_control = false
		if not is_on_floor():
			velocity.x = move_toward(velocity.x, 0, air_res * delta)
		else:
			velocity.x = move_toward(velocity.x, 0, fric * delta)
		return

	# Run modifier
	var current_max_speed = spd
	if Input.is_action_pressed("run") and current_stamina > 0 and velocity.x != 0:
		current_max_speed *= 1.5
		current_stamina -= 10.0 * delta

	# Input Direction
	var direction := Input.get_axis("ui_left", "ui_right")
	if direction and can_control:
		facing_direction = sign(direction)

	# Physics
	if can_control:
		if direction:
			velocity.x = move_toward(velocity.x, direction * current_max_speed, accel * delta)
		else:
			if is_on_floor():
				velocity.x = move_toward(velocity.x, 0, fric * delta)
			else:
				velocity.x = move_toward(velocity.x, 0, air_res * delta)

func _update_state_from_physics() -> void:
	if not asc: return
	if current_health <= 0: return
	if _is_attacking: return

	# Dash in progress
	if _current_state_tag == &"state.dash" or _current_state_tag == &"state.hyperdash":
		if _dash_coyote_timer <= 0:
			asc.cancel_ability_by_tag(_current_state_tag)
		else:
			return

	# Hurt
	if _hurt_timer > 0:
		_try_activate(&"state.hurt")
		return

	# Airborne
	if not is_on_floor():
		if velocity.y < 0:
			_try_activate(&"state.jump")
		else:
			_try_activate(&"state.fall")
		return

	# Ground
	var direction := Input.get_axis("ui_left", "ui_right")
	if direction != 0:
		if Input.is_action_pressed("run") and current_stamina > 0:
			_try_activate(&"state.run")
		else:
			_try_activate(&"state.walk")
	else:
		_try_activate(&"state.idle")

func _try_activate(tag: StringName) -> bool:
	if not asc: return false
	if _current_state_tag == tag: return true
	return asc.try_activate_ability_by_tag(tag)

func _on_tag_changed(tag: StringName, is_present: bool) -> void:
	if is_present:
		if String(tag).begins_with("state.") or String(tag).begins_with("attack."):
			_current_state_tag = tag

func take_damage(amount: int) -> void:
	if current_health <= 0: return
	current_health -= amount
	_hurt_timer = 0.1
	if current_health <= 0:
		die()

func die() -> void:
	_try_activate(&"state.dead")
	emit_signal("died")
	queue_free()

func _process(delta: float) -> void:
	if Engine.is_editor_hint(): return

	# Regeneração de Stamina
	var is_consuming = false
	if _current_state_tag == &"state.run" or _is_attacking or _current_state_tag == &"state.dash":
		is_consuming = true

	if not is_consuming and current_stamina < max_stamina:
		var regen = stamina_regen_rate if stamina_regen_rate > 0 else 12.0
		current_stamina += regen * delta

func reset_dash_timer() -> void:
	_dash_coyote_timer = 0.0
