#*
#* agent_base.gd
#* =============================================================================
#* Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
#*
#* Use of this source code is governed by an MIT-style
#* license that can be found in the LICENSE file or at
#* https://opensource.org/licenses/MIT.
#* =============================================================================
#*
extends CharacterBody2D
## Base agent script integrated with AbilitySystem and LimboAI.

signal death

const MINION_RESOURCE := "res://agents/03_agent_imp.tscn"
const NinjaStar := preload("res://agents/ninja_star/ninja_star.tscn")
const Fireball := preload("res://agents/fireball/fireball.tscn")

var summon_count: int = 0
var _frames_since_facing_update: int = 0
var _is_dead: bool = false
var _moved_this_frame: bool = false

@onready var animation_player: AnimationPlayer = $AnimationPlayer
@onready var asc: ASComponent = $ASComponent
@onready var root: Node2D = $Root
@onready var collision_shape_2d: CollisionShape2D = $CollisionShape2D
@onready var summoning_effect: GPUParticles2D = $FX/Summoned

@export var debug_as: bool = false
var _last_debug_tags: Array = []
var _last_debug_anim: String = ""


func _debug_as() -> void:
	if not is_instance_valid(asc): return
	var current_tags = asc.get_tags()
	current_tags.sort()
	var current_anim = ""
	if is_instance_valid(animation_player):
		current_anim = animation_player.current_animation
	
	if current_tags != _last_debug_tags or current_anim != _last_debug_anim:
		print("[%s] Tags: %s | Anim: %s" % [name, current_tags, current_anim])
		_last_debug_tags = current_tags
		_last_debug_anim = current_anim


func _ready() -> void:
	if OS.get_cmdline_args().has("--playtest-cycle") or OS.get_cmdline_args().has("--debug-ability-system"):
		debug_as = true

	if not is_instance_valid(asc): return
	
	# Link ASC to AnimationPlayer
	asc.set_animation_player(animation_player)
	
	# Ensure abilities are unlocked
	var container = asc.get_container()
	if is_instance_valid(container):
		asc.apply_container(container)
		for ab in container.get_abilities():
			if is_instance_valid(ab):
				asc.unlock_ability_by_resource(ab)
	
	# Attribute registration fallback (headless safety)
	if not asc.has_attribute_by_tag(&"attribute.health"):
		var fallback_set = load("res://resources/attributes/agent_attribute_set.tres")
		if is_instance_valid(fallback_set):
			asc.add_attribute_set(fallback_set)
	
	# ASDelivery initialization for melee/ranged
	_init_deliveries(self)
	
	# Connect ASC signals
	asc.attribute_changed.connect(_on_attribute_changed)
	asc.effects_ready_for_others.connect(_on_effects_ready_for_others)
	asc.ability_activated.connect(_on_ability_activated)
	asc.ability_ended.connect(_on_ability_ended)
	asc.tag_changed.connect(func(_tag, _added): _refresh_animation_state.call_deferred())
	
	_update_conditional_tags()
	_refresh_animation_state.call_deferred()

func die() -> void:
	queue_free()

func _physics_process(_delta: float) -> void:
	if debug_as:
		_debug_as()
		
	# Update moving tag
	if velocity.length() > 5.0:
		if not asc.has_tag(&"state.is_moving"): asc.add_tag(&"state.is_moving")
	else:
		if asc.has_tag(&"state.is_moving"): asc.remove_tag(&"state.is_moving")
		
	_post_physics_process.call_deferred()

func _post_physics_process() -> void:
	if not _moved_this_frame:
		velocity = lerp(velocity, Vector2.ZERO, 0.5)
	_moved_this_frame = false

func move(p_velocity: Vector2) -> void:
	if _is_dead: return
	if asc.has_tag(&"state.dead") or asc.has_tag(&"state.hurt") or asc.has_tag(&"state.attacking") or asc.has_tag(&"state.charging"):
		return
		
	velocity = lerp(velocity, p_velocity, 0.2)
	move_and_slide()
	_moved_this_frame = true

func update_facing() -> void:
	_frames_since_facing_update += 1
	if _frames_since_facing_update > 3:
		face_dir(velocity.x)

func face_dir(dir: float) -> void:
	if dir > 0.0 and root.scale.x < 0.0:
		root.scale.x = 1.0;
		_frames_since_facing_update = 0
	if dir < 0.0 and root.scale.x > 0.0:
		root.scale.x = -1.0;
		_frames_since_facing_update = 0

func get_facing() -> float:
	return signf(root.scale.x)

# --- AbilitySystem Handlers ---

func _update_conditional_tags() -> void:
	if not is_instance_valid(asc): return
	
	var hp = asc.get_attribute_value_by_tag(&"attribute.health")
	
	# Low Health
	if hp > 0 and hp < 40.0:
		if not asc.has_tag(&"state.low_health"): asc.add_tag(&"state.low_health")
	else:
		if asc.has_tag(&"state.low_health"): asc.remove_tag(&"state.low_health")
		
	# Critical Health
	if hp > 0 and hp < 15.0:
		if not asc.has_tag(&"state.critical_health"): asc.add_tag(&"state.critical_health")
	else:
		if asc.has_tag(&"state.critical_health"): asc.remove_tag(&"state.critical_health")
	
	# Full Health
	if hp >= 100.0:
		if not asc.has_tag(&"state.full_health"): asc.add_tag(&"state.full_health")
	else:
		if asc.has_tag(&"state.full_health"): asc.remove_tag(&"state.full_health")


func _init_deliveries(p_node: Node) -> void:
	if p_node is ASDelivery:
		p_node.set_source_component(asc)
		if p_node.get_parent() is Hitbox:
			p_node.level = p_node.get_parent().damage
	
	for child in p_node.get_children():
		_init_deliveries(child)

func _on_attribute_changed(attr: StringName, _old: float, new: float) -> void:
	_update_conditional_tags()
	if attr == &"attribute.health":
		if new <= 0:
			asc.call_deferred(&"try_activate_ability_by_tag", &"state.dead")
		elif new < _old:
			asc.call_deferred(&"try_activate_ability_by_tag", &"state.hurt")
			var effect = load("res://resources/effects/invulnerability_post_hit.tres")
			if is_instance_valid(effect):
				asc.call_deferred(&"apply_effect_by_resource", effect)

func _on_ability_activated(spec: ASAbilitySpec) -> void:
	var tag = spec.get_ability().get_ability_tag()
	if tag == &"state.dead":
		var btplayer = get_node_or_null(^"BTPlayer")
		if is_instance_valid(btplayer):
			btplayer.set_active(false)
		_do_death()

func _on_ability_ended(spec: ASAbilitySpec, _cancelled: bool) -> void:
	var tag = spec.get_ability().get_ability_tag()
	if tag == &"state.hurt":
		var btplayer = get_node_or_null(^"BTPlayer")
		if is_instance_valid(btplayer) and not _is_dead:
			btplayer.set_active(true)
			btplayer.restart()
	
	_refresh_animation_state.call_deferred()


func _refresh_animation_state() -> void:
	if not is_instance_valid(animation_player) or not is_instance_valid(asc):
		return
		
	if _is_dead or asc.has_tag(&"state.dead"):
		if animation_player.has_animation("die") and animation_player.current_animation != "die":
			animation_player.play("die")
		return

	if asc.has_tag(&"state.hurt"):
		return # Let hurt animation finish

	if asc.has_tag(&"state.attacking") or asc.has_tag(&"state.charging"):
		return # Let current intensive action finish

	if asc.has_tag(&"state.is_moving"):
		if animation_player.has_animation("walk") and animation_player.current_animation != "walk":
			animation_player.play("walk")
	else:
		if animation_player.has_animation("idle") and animation_player.current_animation != "idle":
			animation_player.play("idle")

func _on_effects_ready_for_others(_ability_spec: ASAbilitySpec, effects: Array[ASEffect]) -> void:
	var hitbox = get_node_or_null("Root/Hitbox")
	if not is_instance_valid(hitbox): return
	
	for area in hitbox.get_overlapping_areas():
		if area is Hurtbox and area.owner != self:
			var target_asc = area.owner.get_node_or_null("ASComponent")
			if is_instance_valid(target_asc):
				for effect in effects:
					if is_instance_valid(effect):
						var spec = asc.make_outgoing_spec(effect, hitbox.damage, area.owner)
						asc.apply_effect_spec_to_target(spec, target_asc)

# --- Combat & Actions ---

func _do_death() -> void:
	if _is_dead: return
	_is_dead = true
	death.emit()
	root.process_mode = Node.PROCESS_MODE_DISABLED
	collision_shape_2d.set_deferred(&"disabled", true)
	
	if get_tree():
		await get_tree().create_timer(10.0).timeout
		queue_free()

func take_damage(amount: float, _knockback: Vector2) -> void:
	if _is_dead: return
	var effect = load("res://resources/effects/damage_instant.tres")
	if is_instance_valid(effect):
		var spec = asc.make_outgoing_spec(effect, amount)
		asc.apply_effect_spec_to_self(spec)

func get_asc() -> ASComponent:
	return asc

# --- Original Helper Methods ---

func throw_ninja_star() -> void:
	var ninja_star := NinjaStar.instantiate()
	ninja_star.dir = get_facing()
	get_parent().add_child(ninja_star)
	ninja_star.global_position = global_position + Vector2.RIGHT * 100.0 * get_facing()
	if ninja_star.has_method(&"set_source"):
		ninja_star.set_source(asc)

func spit_fire() -> void:
	var fireball := Fireball.instantiate()
	fireball.dir = get_facing()
	get_parent().add_child(fireball)
	fireball.global_position = global_position + Vector2.RIGHT * 100.0 * get_facing()
	if fireball.has_method(&"set_source"):
		fireball.set_source(asc)

func summon_minion(p_position: Vector2) -> void:
	var minion = load(MINION_RESOURCE).instantiate()
	get_parent().add_child(minion)
	minion.position = p_position
	if minion.has_method(&"play_summoning_effect"):
		minion.play_summoning_effect()
	summon_count += 1
	minion.death.connect(func(): summon_count -= 1)

func play_summoning_effect() -> void:
	if is_instance_valid(summoning_effect):
		summoning_effect.emitting = true

func is_good_position(p_position: Vector2) -> bool:
	var space_state := get_world_2d().direct_space_state
	var params := PhysicsPointQueryParameters2D.new()
	params.position = p_position
	params.collision_mask = 1
	var collision := space_state.intersect_point(params)
	return collision.is_empty()

func apply_knockback(knockback: Vector2, frames: int = 10) -> void:
	if knockback.is_zero_approx():
		return
	for i in range(frames):
		move(knockback)
		if is_inside_tree():
			await get_tree().physics_frame
