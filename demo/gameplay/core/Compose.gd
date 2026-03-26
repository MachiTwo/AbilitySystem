@tool
## Container de composição de estados.
##
## Permite combinar múltiplos sub-estados ou comportamentos em um único recurso.
## Útil para definir conjuntos de regras ou variações de estados complexos.
class_name Compose
extends Resource

@export var move_states: Array[State]
@export var attack_states: Array[State]
@export var interactive_states: Array[State]

@export var move_rules : Dictionary = {}
@export var attack_rules : Dictionary = {}
@export var interactive_rules : Dictionary = {}

# Cache para busca rápida por nome
var _states_map: Dictionary = {}
var _initialized: bool = false

func _ready() -> void:
	if Engine.is_editor_hint():
		initialize()
		# Salva apenas se houver mudanças reais para evitar loop infinito de disco
		# Mas como é difícil detectar, vamos salvar apenas se não estiver inicializado?
		# O usuário pediu "automaticamente se salvar no disco".
		# Vamos forçar o save.
		if resource_path != "":
			ResourceSaver.save(self, resource_path)
			print("Compose Index Built & Saved: ", resource_path)

func initialize() -> void:
	# Sempre reconstroi no editor para garantir dados frescos
	if _initialized and not Engine.is_editor_hint(): return
	
	_states_map.clear()
	move_rules.clear()
	attack_rules.clear()
	interactive_rules.clear()

	move_rules = _build_index(move_states)
	attack_rules = _build_index(attack_states)
	interactive_rules = _build_index(interactive_states)

	_initialized = true

func _build_index(states: Array[State]) -> Dictionary:
	var index = {}
	for state in states:
		if not state: continue
		_register_state(state)
		
		var key = state.get_lookup_key()
		if not index.has(key): index[key] = []
		index[key].append(state)
		print("[Compose] Indexed %s -> Key %d" % [state.name, key])
	return index

func _register_state(res: Resource) -> void:
	if res and "name" in res:
		_states_map[res.name] = res

func get_state_by_name(name: String) -> Resource:
	if not _initialized: initialize()
	return _states_map.get(name)

func get_move_states() -> Array[State]:
	return move_states

func get_attack_states() -> Array[State]:
	return attack_states

func get_interactive_states() -> Array[State]:
	return interactive_states
