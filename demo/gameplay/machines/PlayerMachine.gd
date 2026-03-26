## Máquina de Estados especializada para o Jogador.
##
## Processa inputs diretos e gerencia estados complexos de combate e movimento.
## Suporta buffer de inputs, cancelamento de animações e aplicação de buffs.
class_name PlayerMachine
extends BehaviorMachine

@export var stats: CharacterStats = preload("res://player/resources/stats.tres")

@export var fallback_item: ItemData

@export var default_weapon: BehaviorStates.Weapon = BehaviorStates.Weapon.NONE

var current_item: ItemData

signal attack_executed(attack_data: AttackData)
signal item_changed(item: ItemData)

# Queue Logic
var _queued_state: Resource = null
var _combo_intent: bool = false

func _ready() -> void:
	# Inicializa Fallback
	if fallback_item and fallback_item.compose:
		fallback_item.compose.initialize()
		
	# Define item inicial como fallback se não houver outro
	if not current_item:
		current_item = fallback_item

	# Configura estado inicial
	set_context("Physics", BehaviorStates.Physics.GROUND)
	set_context("Motion", BehaviorStates.Motion.IDLE)
	set_context("Attack", BehaviorStates.Attack.NONE)
	
	# Garante que a avaliação inicial ocorra após todos os _ready
	call_deferred("_try_engage_move", true)

func _process(delta: float) -> void:
	super(delta) # Updates time_in_state
	
	if not current_state_res:
		# Watchdog: Se não tem estado, tenta engajar movimento imediatamente
		_try_engage_move(true)
	
	# Verifica se o ataque acabou
	if current_state_res is AttackData:
		var atk = current_state_res as AttackData
		if time_in_state >= atk.duration:
			print("[Time] Duration Reached for ", atk.name, ". Finishing.")
			_finish_attack_state()
			
	# Verifica se o movimento acabou (ex: Dash)
	elif current_state_res is MoveData:
		var mov = current_state_res as MoveData
		if mov.duration > 0 and time_in_state >= mov.duration:
			print("[Time] Duration Reached for ", mov.name, ". Finishing.")
			
			if mov.lock_movement:
				var p = owner as Player
				if p: p.reset_dash_timer()
				# Força reset do contexto para evitar loop imediato (o Player.gd vai atualizar no próximo frame)
				set_context("Motion", BehaviorStates.Motion.IDLE)
				
			change_state(null) # Reseta para reavaliar contexto
			_try_engage_move(true)

func _on_context_updated(category: String, value: int) -> void:
	# 1. Input de Ataque: Se tiver intenção de ataque, TENTA atacar.
	if category == "Attack":
		if value != BehaviorStates.Attack.NONE:
			_try_engage_attack()
		# Se Attack virou NONE, talvez devêssemos voltar pro move?
		# Deixa o fluxo natural seguir.

	# 2. Se estivermos atacando (Contexto Attack != NONE), IGNORA mudanças de Motion/Physics
	# A menos que seja uma reação (handled below) ou o ataque acabou.
	if context.get("Attack", BehaviorStates.Attack.NONE) != BehaviorStates.Attack.NONE:
		pass
	
	# 3. Reação Baseada no Estado Atual
	if current_state_res:
		var reaction = _get_reaction(current_state_res, category)
		_handle_reaction(reaction)
	else:
		# Se não tem estado, sempre tenta engajar (comportamento padrão)
		if category == "Motion" or category == "Physics" or category == "Jump" or category == "Attack":
			_try_engage_move(true)

func change_state(new_res: Resource, preserve_time: bool = false) -> void:
	super(new_res, preserve_time)
	
	# Aplica buffs do novo estado
	if new_res and "buffs" in new_res and not new_res.buffs.is_empty():
		print("[PlayerMachine] Applying Buffs from ", new_res.name)
		for buff in new_res.buffs:
			# TODO: Integrar com sistema de Buffs real
			print(" - Buff Applied: ", buff)

func _finish_attack_state() -> void:
	if _queued_state:
		change_state(_queued_state)
		_queued_state = null
		emit_signal("attack_executed", current_state_res)
	else:
		# Se não tem nada na fila, volta pro fluxo normal (provavelmente Idle/Move)
		# Força reavaliação do contexto
		change_state(null)
		set_context("Attack", BehaviorStates.Attack.NONE)

func _unhandled_input(event: InputEvent) -> void:
	# COMBO FIX: Captura input direto para garantir intenção de combo
	# mesmo que o contexto não mude (ex: spamando botão)
	if current_state_res is AttackData:
		if event.is_action_pressed("attack_light") or event.is_action_pressed("attack_heavy"):
			_combo_intent = true
			print("[PlayerMachine] Combo Intent Registered via Input!")
			# Tenta agendar imediatamente
			_try_engage_attack()

func _try_engage_attack(is_adapting: bool = false) -> void:
	# Coleta candidatos do item atual e do fallback (Usando Hash Map O(1))
	var candidates: Array[Resource] = []
	var key = context.get("Attack", BehaviorStates.Attack.ANY)
	print("[DEBUG] Try Engage Attack. Key: ", key)
	
	if current_item and current_item.compose:
		var found = current_item.compose.attack_rules.get(key, [])
		print("[DEBUG] Candidates from Current Item (Key ", key, "): ", found.size())
		candidates.append_array(found)
		if key != 0: 
			var any_found = current_item.compose.attack_rules.get(0, [])
			print("[DEBUG] Candidates from Current Item (ANY): ", any_found.size())
			candidates.append_array(any_found) # ANY
		
	if fallback_item and fallback_item.compose:
		candidates.append_array(fallback_item.compose.attack_rules.get(key, []))
		if key != 0: candidates.append_array(fallback_item.compose.attack_rules.get(0, [])) # ANY
		
	if candidates.is_empty(): return
	
	# Combo Logic: Se já estamos atacando, verifica se pode encadear
	if current_state_res and current_state_res is AttackData:
		var current_attack = current_state_res as AttackData
		
		# Se tem próximo combo e estamos na janela de tempo
		if current_attack.next_combo_state:
			# COMBO FIX: Só agenda se houver INTENÇÃO explícita (Input pressionado durante o ataque)
			if _combo_intent:
				_queued_state = current_attack.next_combo_state
				_combo_intent = false # Consome a intenção
				print("Queued Next Attack: ", _queued_state.name)
			return
			
	# Se não estamos atacando, inicia ataque normalmente
	_try_engage_from_list(candidates, is_adapting)

func _try_engage_move(is_adapting: bool = false) -> void:
	# Coleta candidatos do item atual e do fallback (Usando Hash Map O(1))
	var candidates: Array[Resource] = []
	var key = context.get("Motion", BehaviorStates.Motion.IDLE)
	
	if current_item and current_item.compose:
		candidates.append_array(current_item.compose.move_rules.get(key, []))
		if key != 0: candidates.append_array(current_item.compose.move_rules.get(0, [])) # ANY
		
	if fallback_item and fallback_item.compose:
		candidates.append_array(fallback_item.compose.move_rules.get(key, []))
		if key != 0: candidates.append_array(fallback_item.compose.move_rules.get(0, [])) # ANY
		
	# DEBUG JUMP
	if context.get("Jump", 0) != 0:
		print("[PlayerMachine] Jump Context Active: ", context.get("Jump"), " Key: ", key)
		print("Candidates found: ", candidates.size())
		for c in candidates: print("- Candidate: ", c.name)

	# DEBUG DASH
	if key == BehaviorStates.Motion.DASH:
		print("[PlayerMachine] Dash Context Active! Candidates: ", candidates.size())
		for c in candidates: print("- Candidate: ", c.name)

	if candidates.is_empty(): return
	
	# Se estamos carregando um ataque (Charged), não move
	if current_state_res and current_state_res is AttackData:
		if "is_charged" in current_state_res and current_state_res.is_charged:
			return
			
	# Movimento tem prioridade menor que ataque, então só troca se não estiver atacando
	# OU se o ataque permitir cancelamento (mas isso é handled por _handle_reaction?)
	# Por enquanto, vamos tentar trocar se não tiver estado ou se o estado atual for de movimento
	
	# Se já estamos num estado de ataque que não acabou, não interrompe (a menos que seja reação)
	if current_state_res and current_state_res is AttackData and not is_adapting:
		return
		
	# Se estamos num estado de movimento Travado (ex: Dash), não interrompe
	if current_state_res and current_state_res is MoveData:
		if current_state_res.duration > 0 and current_state_res.lock_movement:
			# EXCEÇÃO: Hyper Dash (Permite pular durante o dash)
			# Se o contexto de Jump mudou para algo que não seja NONE, permitimos a troca
			if context.get("Jump") != BehaviorStates.Jump.NONE:
				pass # Permite continuar para _try_engage_generic
			else:
				# CANCEL WINDOW CHECK: Se já passou do tempo mínimo de cancelamento, permite sair
				if current_state_res.cancel_min_time > 0 and time_in_state >= current_state_res.cancel_min_time:
					print("[PlayerMachine] Cancel Window Open. Interrupting Locked State.")
					pass # Permite continuar
				else:
					return
		
	_try_engage_from_list(candidates, is_adapting)

func _try_engage_from_list(candidates: Array[Resource], is_adapting: bool) -> void:
	var best = find_best_match(candidates)

	if best:
		var preserve = false
		if is_adapting and current_state_res and best.name == current_state_res.name:
			preserve = true
		
		# Evita reiniciar o mesmo estado de movimento se já estiver nele
		if current_state_res and best == current_state_res:
			return

		change_state(best, preserve)
		
		if best is AttackData:
			emit_signal("attack_executed", best)
			
	elif is_adapting:
		change_state(null) # Falhou a adaptação

func _handle_reaction(reaction: int) -> void:
	match reaction:
		BehaviorStates.Reaction.CANCEL: change_state(null)
		BehaviorStates.Reaction.ADAPT: 
			# Tenta adaptar no compose apropriado. 
			# Se for ataque, tenta ataque. Se for movimento, tenta movimento.
			if current_state_res is AttackData:
				_try_engage_attack(true)
			elif current_state_res is MoveData:
				_try_engage_move(true)
				
		BehaviorStates.Reaction.FINISH:
			if current_state_res: current_state_res.loop = false

# Helper para ler propriedades de forma segura
func _get_reaction(res: Resource, category: String) -> int:
	# Mapeia o nome da categoria para a propriedade de reação
	var prop = "on_" + category.to_lower() + "_change"
	var val = res.get(prop)
	if val != null: return val
	return BehaviorStates.Reaction.ADAPT

func get_resource_value(type: int) -> float:
	# Tenta pegar do Owner (Player)
	var p = owner as Player
	if not p: return 9999.0
	
	match type:
		BehaviorStates.CostType.STAMINA: return p.current_stamina
		BehaviorStates.CostType.HEALTH: return float(p.current_health)
		_: return 9999.0

func set_current_item(item: ItemData) -> void:
	current_item = item
	if current_item and current_item.compose:
		current_item.compose.initialize()
	
	# Se item for nulo, volta pro fallback
	if not current_item:
		current_item = fallback_item
		
	# Reavalia estado imediatamente
	_try_engage_move(true)
	
	emit_signal("item_changed", current_item)
