@abstract
## Classe base para todos os Estados do sistema.
##
## Contém TODAS as propriedades possíveis para um estado (Movimento, Ataque, Visual, etc).
## Isso permite "Duck Typing" e estados híbridos (ex: Ataque que também é Dash).
class_name State
extends Resource

@export_group("Identity & Visuals")
## Nome de exibição do estado (para debug e logs).
@export var name: String = "State"
## Textura/SpriteSheet usada neste estado.
@export var texture: Texture2D
## Número de colunas no SpriteSheet (frames horizontais).
@export var hframes: int = 1
## Número de linhas no SpriteSheet (frames verticais).
@export var vframes: int = 1
## Recurso de Animação (se usar AnimationPlayer).
@export var animation_res: Animation
## Se a animação deve rodar em loop.
@export var loop: bool = false
## Multiplicador de velocidade da animação.
@export var speed: float = 1.0
## Som a ser tocado ao entrar no estado.
@export var sound: AudioStream
## Cor de debug para gizmos ou visualização.
@export var debug_color: Color = Color.WHITE

@export_group("Movement Logic")
## Multiplicador de velocidade de movimento do personagem.
@export var speed_multiplier: float = 1.0
## Duração fixa do estado em segundos. Se > 0, o estado termina automaticamente após esse tempo.
@export var duration: float = 0.0 
## Se verdadeiro, ignora input direcional do jogador (trava movimento).
@export var lock_movement: bool = false 
## Se verdadeiro, o estado é cancelado se o personagem colidir com uma parede.
@export var cancel_on_wall: bool = false
## Se verdadeiro, desativa a gravidade durante este estado.
@export var ignore_gravity: bool = false

@export_group("Physics Parameters")
## Aceleração personalizada. Se 0, usa o padrão do Player.
@export var acceleration: float = 0.0 
## Fricção personalizada.
@export var friction: float = 0.0
## Resistência do ar personalizada.
@export var air_resistance: float = 0.0
## Força de pulo aplicada ao ENTRAR no estado. Se > 0, impulsiona verticalmente.
@export var jump_force: float = 0.0 

@export_group("Combat (Melee)")
## Dano base causado por este estado (se for um ataque).
@export var damage: int = 0

@export_group("Cooldowns")
## Tempo de recarga deste estado específico em segundos.
@export var cooldown: float = 0.0
## Categoria de contexto para aplicar cooldown global (ex: MOTION para cooldown de Dash).
@export var context_cooldown_filter: BehaviorStates.ContextFilter = BehaviorStates.ContextFilter.NONE
## Tempo de cooldown a ser aplicado na categoria de contexto selecionada.
@export var context_cooldown_time: float = 0.0

@export_group("Requirements")
## Ponto pivô da área de efeito (offset).
@export var area_pivot: Vector2 = Vector2.ZERO
## Tamanho da área de efeito (Hitbox/Hurtbox).
@export var area_size: Vector2 = Vector2.ZERO
## Se verdadeiro, mantém o momentum atual ao entrar no estado.
@export var preserve_momentum: bool = false

@export_group("Combat (Ranged)")
## Cena do projétil a ser instanciada.
@export var projectile_scene: PackedScene
## Velocidade do projétil.
@export var projectile_speed: float = 0.0
## Quantidade de projéteis disparados.
@export var projectile_count: int = 0
## Dispersão angular dos projéteis (em graus).
@export var projectile_spread: float = 0.0
## Offset de spawn do projétil relativo ao personagem.
@export var spawn_offset: Vector2 = Vector2.ZERO

@export_group("Combo System")
## Identificador do passo atual no combo (ex: STEP_1, FINISHER).
@export var combo_step: BehaviorStates.ComboStep = BehaviorStates.ComboStep.NONE
## Próximo estado possível no combo (se houver input).
@export var next_combo_state: State
## Tempo a partir do qual o input para o próximo combo é aceito.
@export var combo_window_start: float = 0.0

@export_group("Charged Attack")
## Indica se este é um ataque carregado.
@export var is_charged: bool = false
## Tempo mínimo de carga para ativar.
@export var min_charge_time: float = 0.0
## Tempo máximo de carga (auto-release).
@export var max_charge_time: float = 0.0
## Multiplicador de dano quando totalmente carregado.
@export var fully_charged_damage_multiplier: float = 1.0

@export_group("Buffs & Debuffs")
## Lista de buffs (Resources) a serem aplicados ao entrar neste estado.
@export var buffs: Array[Resource]

@export_group("Timing & Windows")
## Tempo mínimo antes que o estado possa ser cancelado por outro (exceto reações forçadas).
@export var cancel_min_time: float = 0.0
## Se verdadeiro, permite bufferizar inputs durante este estado.
@export var enable_buffering: bool = false
## Tempo a partir do qual o buffer de input começa a funcionar.
@export var buffer_window_start: float = 0.0

@export_group("Costs")
## Tipo de recurso consumido (Stamina, Mana, Health).
@export var cost_type: BehaviorStates.CostType = BehaviorStates.CostType.NONE
## Quantidade do recurso consumida.
@export var cost_amount: int = 0

## Regra para quando o recurso for insuficiente.
@export var on_insufficient_resource: BehaviorStates.LowResourceRule = BehaviorStates.LowResourceRule.IGNORE_COMMAND


@export_category("Filters (Requirements)")
## Prioridade manual para desempate (maior vence).
@export var priority_override: int = 0
## Lista de estados anteriores permitidos. Se vazio, aceita qualquer um.
@export var previous_states: Array[State] = []
## Requisitos de entrada baseados no Contexto (Motion, Jump, Attack, etc).
@export var entry_requirements: Dictionary = {
	"motion": BehaviorStates.Motion.ANY,
	"jump": BehaviorStates.Jump.ANY,
	"attack": BehaviorStates.Attack.ANY,
	"physics": BehaviorStates.Physics.ANY,
	"status": BehaviorStates.Status.ANY,
	"effect": BehaviorStates.Effect.ANY
}
## Requisitos para MANTER o estado ativo. Se falhar, o estado é interrompido.
@export var maintenance_requirements: Dictionary = {
	"motion": BehaviorStates.Motion.ANY,
	"jump": BehaviorStates.Jump.ANY,
	"attack": BehaviorStates.Attack.ANY,
	"physics": BehaviorStates.Physics.ANY,
	"status": BehaviorStates.Status.ANY,
	"effect": BehaviorStates.Effect.ANY
}

@export_group("Resource Requirements")
## Vida mínima necessária para entrar no estado.
@export var req_min_hp: float = 0.0
## Vida máxima permitida para entrar no estado.
@export var req_max_hp: float = 100.0

@export_category("Reaction Rules")
## Reação ao mudar o contexto de Física (ex: cair no chão).
@export var on_physics_change: BehaviorStates.Reaction = BehaviorStates.Reaction.ADAPT
## Reação ao mudar a arma equipada.
@export var on_weapon_change: BehaviorStates.Reaction = BehaviorStates.Reaction.ADAPT
## Reação ao mudar o contexto de Movimento.
@export var on_motion_change: BehaviorStates.Reaction = BehaviorStates.Reaction.ADAPT
## Reação ao mudar o contexto de Ataque.
@export var on_attack_change: BehaviorStates.Reaction = BehaviorStates.Reaction.ADAPT
## Reação ao receber dano.
@export var on_take_damage: BehaviorStates.Reaction = BehaviorStates.Reaction.ADAPT

## Retorna a chave de busca para o HashMap (Otimização).
## Usado pelo Compose.gd para indexar este estado.
func get_lookup_key() -> int:
	if self is MoveData: 
		var m = entry_requirements.get("motion", 0)
		# Se for EXCEPT_DASH (5), trata como genérico (0) para cair no bucket ANY
		if m == 5: return 0 
		return m
	if self is AttackData: 
		var a = entry_requirements.get("attack", 0)
		# print("[State] %s is AttackData. Key: %d" % [name, a])
		return a
	return 0 # ANY
