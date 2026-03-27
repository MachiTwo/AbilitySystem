class_name ItemData
extends Resource

@export var display_name: String = "Item"
@export var icon: Texture2D

@export_group("Behavior")
## Composição de estados (Move, Attack, Interactive) associada a este item.
@export var compose: Compose

## Habilidade baseada no Ability System que este item desbloqueia (Opcional, se quiser dar uma habilidade única).
@export var ability: ASAbility

## Tag condicional que este item concede para habilitar habilidades específicas (ex: Weapon.Sword).
@export var weapon_tag: StringName = &"weapon.unarmed"

## Dicionário de modificadores de contexto ao equipar este item.
## Ex: { "Weapon": BehaviorStates.Weapon.KATANA }
@export var context_modifiers: Dictionary = {
	"Weapon": 0 # BehaviorStates.Weapon.ANY/NONE
}

func get_context_modifier(category: String) -> int:
	if context_modifiers.has(category):
		return context_modifiers[category]
	return -1 # Not found
