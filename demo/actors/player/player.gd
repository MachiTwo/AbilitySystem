extends CharacterBody2D

@onready var asc: ASComponent = $ASComponent
@onready var sprite: AnimatedSprite2D = $AnimatedSprite2D
@onready var attack_area: Area2D = $Area2D
@onready var audio_player: AudioStreamPlayer2D = $AudioStreamPlayer2D

var is_dead = false

func _ready() -> void:
	print("DEBUG: Player _ready chamado")
	asc.set_animation_player(sprite)
	asc.set_audio_player(audio_player)
	
	asc.attribute_changed.connect(_on_attribute_changed)
	asc.ability_activated.connect(_on_ability_activated)
	asc.ability_ended.connect(_on_ability_ended)
	asc.effects_applied_to_self.connect(_on_effects_applied_to_self)
	asc.effects_ready_for_others.connect(_on_effects_ready_for_others)
	
	await get_tree().process_frame
	
	# Inicializa em Idle
	asc.try_activate_ability_by_tag(&"ability.skill.idle")

func _physics_process(delta: float) -> void:
	if is_dead: return
	
	var is_attacking = asc.has_tag(&"state.attacking")

	if Input.is_action_just_pressed("attack") and not is_attacking:
		var result = asc.try_activate_ability_by_tag(&"ability.skill.attack")
		print("DEBUG: try_activate attack retornou: ", result)

	var direction := Input.get_vector("move_left", "move_right", "move_up", "move_down")

	if direction:
		# Can -> Cancel -> Try: Walk
		if not asc.has_tag(&"ability.skill.walk"):
			if asc.can_activate_ability_by_tag(&"ability.skill.walk"):
				asc.cancel_ability_by_tag(&"ability.skill.idle")
				asc.try_activate_ability_by_tag(&"ability.skill.walk")
	else:
		# Can -> Cancel -> Try: Idle
		if not asc.has_tag(&"ability.skill.idle"):
			if asc.can_activate_ability_by_tag(&"ability.skill.idle"):
				asc.cancel_ability_by_tag(&"ability.skill.walk")
				asc.try_activate_ability_by_tag(&"ability.skill.idle")

	# Só aplica velocidade se o ASC permitiu o estado de movimento (Walk ativo)
	if asc.has_tag(&"ability.skill.walk") and not is_attacking:
		var move_speed = asc.get_attribute_value_by_tag(&"move_speed")
		velocity = direction * move_speed
		if direction.x != 0:
			sprite.flip_h = direction.x < 0
			attack_area.scale.x = 1 if direction.x > 0 else -1
	else:
		var move_speed = asc.get_attribute_value_by_tag(&"move_speed")
		velocity = velocity.move_toward(Vector2.ZERO, move_speed * delta * 10)

	move_and_slide()

func _on_ability_activated(spec: ASAbilitySpec):
	var ability = spec.get_ability()
	var tag = ability.get_ability_tag()
	print("DEBUG: Ability Activated: ", tag)
	match tag:
		&"character.state.dead":
			is_dead = true

func _on_ability_ended(spec: ASAbilitySpec, _cancelled: bool):
	var tag = spec.get_ability().get_ability_tag()
	print("DEBUG: Ability Ended: ", tag, " Cancelled: ", _cancelled)
	
	# Quando o ataque (ou outra habilidade temporária) termina, precisamos garantir
	# que voltamos para a animação correta (Idle ou Walk).
	if tag == &"ability.skill.attack" or tag == &"character.state.hurt":
		_refresh_base_state()

func _refresh_base_state():
	# Se ainda estivermos atacando, não devemos forçar idle/walk
	if asc.has_tag(&"state.attacking"):
		return
		
	# Força a atualização do estado visual baseado no movimento atual.
	var direction := Input.get_vector("move_left", "move_right", "move_up", "move_down")
	if direction:
		if asc.has_tag(&"ability.skill.walk"):
			sprite.play("walk")
	else:
		if asc.has_tag(&"ability.skill.idle"):
			sprite.play("idle")

func _on_effects_ready_for_others(ability_spec: ASAbilitySpec, effects: Array[ASEffect]):
	print("DEBUG: Effects ready for others: ", effects.size())
	# Se a habilidade for de ataque, aplicamos os efeitos na área
	if ability_spec.get_ability().get_ability_tag() == &"ability.skill.attack":
		_do_attack_hit(effects)

func _on_effects_applied_to_self(ability_spec: ASAbilitySpec, effect_specs: Array[ASEffectSpec]):
	print("DEBUG: Effects applied to self: ", effect_specs.size())
	for spec in effect_specs:
		print("  - Applied: ", spec.get_effect().get_effect_name())

func _do_attack_hit(effects: Array[ASEffect]):
	print("DEBUG: Doing attack hit check with provided effects")
	for area in attack_area.get_overlapping_areas():
		var parent = area.get_parent()
		# Verificamos se o pai tem um ASC (pode ser Enemy ou outro destructible)
		var enemy_asc = parent.get_node_or_null("ASComponent")
		if enemy_asc:
			print("DEBUG: Applying ability effects to: ", parent.name)
			for effect in effects:
				var spec = asc.make_outgoing_spec(effect, 1.0, parent)
				asc.apply_effect_spec_to_target(spec, enemy_asc)

func _on_attribute_changed(attr: StringName, old: float, new: float):
	print("DEBUG: Attribute Changed: ", attr, " Old: ", old, " New: ", new)
	if attr == &"Health":
		if new <= 0 and not is_dead:
			asc.try_activate_ability_by_tag(&"character.state.dead")
			_die_cleanup()
		elif new < old:
			asc.try_activate_ability_by_tag(&"character.state.hurt")

func _die_cleanup():
	print("DEBUG: Player died")
	collision_layer = 0
	await get_tree().create_timer(2.0).timeout
	get_tree().reload_current_scene()

func _on_as_component_ability_activated(ability_spec: ASAbilitySpec) -> void:
	print("DEBUG: Signal ability_activated received for ", ability_spec.get_ability().get_ability_tag())

func _on_as_component_ability_failed(ability_name: StringName, reason: String) -> void:
	print("DEBUG: Ability Failed: ", ability_name, " Reason: ", reason)

func _on_as_component_attribute_changed(attribute_name: StringName, old_value: float, new_value: float) -> void:
	pass

func _on_as_component_cooldown_ended(ability_tag: StringName) -> void:
	pass

func _on_as_component_cooldown_started(ability_tag: StringName, duration: float) -> void:
	pass

func _on_as_component_effect_applied(effect_spec: ASEffectSpec) -> void:
	pass

func _on_as_component_effect_removed(effect_spec: ASEffectSpec) -> void:
	pass

func _on_as_component_tag_changed(tag_name: StringName, is_present: bool) -> void:
	pass

func _on_as_component_tag_event_received(event_tag: StringName, data: Dictionary) -> void:
	pass




func _on_area_2d_body_entered(body: Node2D) -> void:
	pass # Replace with function body.
