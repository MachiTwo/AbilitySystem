extends CharacterBody2D
class_name Enemy

@onready var asc: ASComponent = $ASComponent
@onready var sprite: AnimatedSprite2D = $AnimatedSprite2D
@onready var audio_player: AudioStreamPlayer2D = $AudioStreamPlayer2D

var is_dead = false
var target: Node2D = null
var attack_range = 40.0
var chase_range = 200.0

func _ready() -> void:
	# Setup Audio Slot
	asc.set_audio_player(audio_player)
	
	# Connect signals
	asc.attribute_changed.connect(_on_attribute_changed)
	asc.ability_activated.connect(_on_ability_activated)
	asc.effects_ready_for_others.connect(_on_effects_ready_for_others)
	asc.effects_applied_to_self.connect(_on_effects_applied_to_self)
	
	# Find player
	target = get_tree().get_first_node_in_group("player")
	if not target:
		# Fallback if group not set
		target = get_parent().get_node_or_null("Player")

func _physics_process(delta: float) -> void:
	if is_dead: return
	
	if asc.has_tag(&"state.attacking"):
		velocity = velocity.move_toward(Vector2.ZERO, 200 * delta)
		move_and_slide()
		return

	if target:
		var dist = global_position.distance_to(target.global_position)
		if dist < attack_range:
			asc.try_activate_ability_by_tag(&"ability.skill.attack")
			velocity = Vector2.ZERO
		elif dist < chase_range:
			var dir = global_position.direction_to(target.global_position)
			var speed = asc.get_attribute_value_by_tag(&"move_speed")
			velocity = dir * speed
			sprite.flip_h = dir.x < 0
			if not asc.has_tag(&"ability.skill.walk"):
				asc.try_activate_ability_by_tag(&"ability.skill.walk")
		else:
			_idle_logic(delta)
	else:
		_idle_logic(delta)

	move_and_slide()

func _idle_logic(delta):
	velocity = velocity.move_toward(Vector2.ZERO, 200 * delta)
	if not asc.has_tag(&"ability.skill.idle") and not asc.has_tag(&"state.attacking"):
		asc.try_activate_ability_by_tag(&"ability.skill.idle")

func _on_ability_activated(spec: ASAbilitySpec):
	var ability = spec.get_ability()
	match ability.get_ability_tag():
		&"ability.skill.walk":
			sprite.play("walk")
		&"ability.skill.idle":
			sprite.play("idle")
		&"ability.skill.attack":
			sprite.play("attack") # Assuming enemy has attack animation

func _on_effects_ready_for_others(ability_spec: ASAbilitySpec, effects: Array[ASEffect]):
	# Simple proximity hit for enemy
	if target and global_position.distance_to(target.global_position) < attack_range + 10:
		var target_asc = target.get_node_or_null("ASComponent")
		if target_asc:
			for effect in effects:
				var spec = asc.make_outgoing_spec(effect, 1.0, target)
				asc.apply_effect_spec_to_target(spec, target_asc)

func _on_effects_applied_to_self(ability_spec: ASAbilitySpec, effect_specs: Array[ASEffectSpec]):
	pass

func _on_attribute_changed(attr: StringName, old: float, new: float):
	if attr == &"Health":
		if new <= 0 and not is_dead:
			_die()
		elif new < old:
			_play_hurt()

func _play_hurt():
	if sprite.sprite_frames.has_animation("hurt"):
		sprite.play("hurt")
		await sprite.animation_finished
		if not is_dead and not asc.has_tag(&"state.attacking"):
			sprite.play("idle")

func _die():
	is_dead = true
	if sprite.sprite_frames.has_animation("dead"):
		sprite.play("dead")
	
	# Disable collisions
	collision_layer = 0
	collision_mask = 0
	
	# Try to stop current logic
	asc.cancel_all_abilities()
	
	await get_tree().create_timer(3.0).timeout
	queue_free()
