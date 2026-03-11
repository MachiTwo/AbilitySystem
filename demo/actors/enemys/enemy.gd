extends CharacterBody2D
class_name Enemy

@onready var asc: AbilitySystemComponent = $AbilitySystemComponent
@onready var sprite: AnimatedSprite2D = $AnimatedSprite2D
@onready var audio_player: AudioStreamPlayer2D = $AudioStreamPlayer2D

var is_dead = false

func _ready() -> void:
	# Setup Audio Slot
	asc.set_audio_player(audio_player)
	
	# Connect signals
	asc.attribute_changed.connect(_on_attribute_changed)
	asc.ability_activated.connect(_on_ability_activated)


func _physics_process(delta: float) -> void:
	if is_dead: return
	
	# Basic AI: just idle for now, but via ability
	if velocity.length() > 0:
		if not asc.has_tag(&"ability.skill.walk"):
			asc.try_activate_ability_by_tag(&"ability.skill.walk")
	else:
		if not asc.has_tag(&"ability.skill.idle"):
			asc.try_activate_ability_by_tag(&"ability.skill.idle")

	move_and_slide()

func _on_ability_activated(spec: AbilitySystemAbilitySpec):
	var ability = spec.get_ability()
	# print("DEBUG: Enemy Ability Activated: ", ability.get_ability_tag())
	match ability.get_ability_tag():
		&"ability.skill.walk":
			sprite.play("walk")
		&"ability.skill.idle":
			sprite.play("idle")

func _on_attribute_changed(attr: StringName, old: float, new: float):
	if attr == &"Health":
		print("DEBUG: Enemy Health Changed: ", new)
		if new <= 0 and not is_dead:
			_die()
		elif new < old:
			_play_hurt()

func _play_hurt():
	print("DEBUG: Enemy Hurt")
	if sprite.sprite_frames.has_animation("hurt"):
		sprite.play("hurt")
		await sprite.animation_finished
		if not is_dead:
			sprite.play("idle")

func _die():
	print("DEBUG: Enemy Died")
	is_dead = true
	if sprite.sprite_frames.has_animation("dead"):
		sprite.play("dead")
	
	# Disable collisions
	collision_layer = 0
	collision_mask = 0
	
	await get_tree().create_timer(3.0).timeout
	queue_free()
