## Dicionário de Tipos Global
##
## Adicionar um item aqui atualiza automaticamente todos os Resources do jogo.
class_name BehaviorStates
extends Node

const ANY = 0

enum Motion { ANY, IDLE, WALK, RUN, DASH, EXCEPT_DASH }
enum Jump { ANY, NONE, LOW, HIGH, FALL }
enum Attack { ANY, NONE, FAST, NORMAL, CHARGED, SPECIAL }
enum Physics { ANY, GROUND, AIR, WATER, EXCEPT_GROUND, EXCEPT_AIR, EXCEPT_WATER }
enum Effect { ANY, NONE, FIRE, ICE, POISON, ELECTRIC }
enum Weapon { ANY, NONE, KATANA, BOW, EXCEPT_NONE }
enum Armor { ANY, NONE, IRON, STEEL, GOLD, DIAMOND }
enum Stance { ANY, STAND, CROUCH, BLOCK, CLIMB, COVER }
enum Tier { ANY, BASE, UPGRADED, MASTER, CORRUPTED }
enum GameState { ANY, PLAYING, PAUSED, CUTSCENE, MENU }
enum StateType { MOVE, ATTACK, INTERACTIVE, GAME }
enum Status { ANY, NORMAL, STUNNED, INVULNERABLE, SUPER_ARMOR, DEAD, HURT }
enum InputSource { ANY, PLAYER, AI, CINEMATIC, FORCE }
enum EnvType { ANY, OPEN, TIGHT_CORRIDOR, LEDGE, WATER_SURFACE }

# Regras de Reação
enum Reaction {
	IGNORE,     # Mantém o estado atual
	CANCEL,     # Interrompe o estado
	ADAPT,      # Tenta adaptar para o novo contexto
	FINISH      # Impede loop/combo
}

enum CostType { NONE, STAMINA, MANA, HEALTH, AMMO }
enum LowResourceRule { IGNORE_COMMAND, EXECUTE_WEAK, CONSUME_HEALTH }

enum ComboStep { NONE, STEP_1, STEP_2, STEP_3, STEP_4, FINISHER }

enum ContextFilter { NONE, MOTION, JUMP, ATTACK, PHYSICS, STATUS, WEAPON }

func _ready() -> void:
	process_mode = Node.PROCESS_MODE_ALWAYS # Importante para funcionar durante Pause

## Verifica se um valor requerido (do Resource) satisfaz o contexto atual.
## Suporta lógica de "ANY" e "EXCEPT_".
static func check_match(category: String, req_val: int, ctx_val: int) -> bool:
	# 1. ANY sempre aceita (exceto se o contexto for inválido, mas assumimos contexto válido)
	if req_val == ANY:
		return true
		
	# 2. Match Exato
	if req_val == ctx_val:
		return true
		
	# 3. Lógica de Exclusão (EXCEPT_*)
	if category == "Physics":
		match req_val:
			Physics.EXCEPT_WATER: return ctx_val != Physics.WATER
			Physics.EXCEPT_GROUND: return ctx_val != Physics.GROUND
			Physics.EXCEPT_AIR: return ctx_val != Physics.AIR
			
	elif category == "Weapon":
		match req_val:
			Weapon.EXCEPT_NONE: return ctx_val != Weapon.NONE

	elif category == "Motion":
		match req_val:
			Motion.EXCEPT_DASH: return ctx_val != Motion.DASH
			
	# Adicione outros casos conforme necessário
	
	return false
