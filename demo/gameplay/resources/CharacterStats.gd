class_name CharacterStats
extends Resource

signal stats_changed

@export_group("Vitals")
@export var max_health: int = 100
@export var max_stamina: float = 100.0
@export var stamina_regen_rate: float = 12.0

@export_group("Movement")
@export var max_speed: float = 230.0
@export var default_acceleration: float = 1200.0
@export var default_friction: float = 1000.0
@export var default_air_resistance: float = 200.0

@export_group("Jump")
@export var jump_force: float = -500.0
@export var gravity_scale: float = 1.0
@export var coyote_time: float = 0.15
@export var jump_buffer_time: float = 0.1
