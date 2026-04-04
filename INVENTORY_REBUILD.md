# Reconstrução do Sistema de Inventário

## Contexto

A pasta `gameplay` foi acidentalmente deletada, levando consigo todo o sistema de inventário, item e hotbar. Este documento descreve a reconstrução completa em GDScript.

## Arquivos Reconstruídos em `demo/inventory/`

### 1. **Item.gd** (Resource)

- Propriedades: `id`, `name`, `description`, `icon`, `max_stack`, `quantity`
- Métodos principais:
  - `can_stack_with(item: Item)` - verifica compatibilidade
  - `add_quantity(amount)` - adiciona quantidade com stack check
  - `is_equipment()` - detecta se é equipável
  - `is_consumable()` - detecta se é consumível
  - `get_display_name()` - retorna nome para UI

### 2. **Inventory.gd** (Node)

- 20 slots (configurável via `max_slots`)
- Métodos principais:
  - `add_item(item: Item)` - adiciona com auto-stack
  - `remove_item(slot: int)` - remove item do slot
  - `remove_item_quantity(item_id: String, amount: int)` - remove quantidade
  - `get_item(slot: int)` - obtém item do slot
  - `get_item_by_id(item_id: String)` - encontra por ID
  - `has_item(item_id: String)` - verifica existência
  - `get_free_slots()` - conta slots vazios
  - `is_full()` - verifica se cheio
- Sinais emitidos:
  - `item_added(item: Item, slot: int)`
  - `item_removed(item: Item, slot: int)`
  - `item_quantity_changed(item: Item, slot: int, new_quantity: int)`
  - `inventory_full()`

### 3. **Hotbar.gd** (Node)

- 5 slots (configurável via `HOTBAR_SIZE`)
- Quick access para itens equipados
- Métodos principais:
  - `set_inventory(inv: Inventory)` - conecta ao inventário
  - `equip_item(inventory_slot: int)` - equipa de slot livre
  - `equip_item_to_slot(inventory_slot: int, hotbar_slot: int)` - equipa em slot específico
  - `unequip_item(hotbar_slot: int)` - remove do hotbar
  - `select_slot(slot: int)` - seleciona hotbar slot
  - `get_selected_item()` - retorna item selecionado
  - `get_item(slot: int)` - obtém item do slot
  - `is_slot_empty(slot: int)` - verifica se vazio
- Sinais emitidos:
  - `item_equipped(item: Item, slot: int)`
  - `item_unequipped(slot: int)`
  - `selection_changed(slot: int)`

### 4. **AttackData.gd** (Resource)

- Configuração de ataque/habilidade
- Propriedades: `id`, `name`, `description`, `damage`, `damage_type`, `cooldown_seconds`, `stamina_cost`, `knockback_force`, `range_meters`, `animation_name`, `attack_tags`
- Métodos: `has_tag()`, `add_tag()`, `remove_tag()`, `is_melee()`, `is_ranged()`, `can_execute(stamina)`

### 5. **MoveData.gd** (Resource)

- Configuração de movimento
- Propriedades: `max_speed`, `acceleration`, `friction`, `rotation_speed`, `jump_force`, `jump_stamina_cost`, `air_control`, `gravity_scale`, `dash_speed`, `dash_duration_ms`, `dash_stamina_cost`, `dash_cooldown_seconds`, `max_fall_speed`, `fall_damage_threshold`, `move_tags`
- Métodos: `has_tag()`, `add_tag()`, `remove_tag()`, `can_jump(stamina)`, `can_dash(stamina)`, `is_fast_movement()`

## Arquivos Corrigidos

### player.gd

- ✅ Inicialização de hotbar (cria se não existir)
- ✅ Conecta sinal `selection_changed` corretamente
- ✅ Método `_on_hotbar_selection_changed()` agora recebe `slot: int`
- ✅ Métodos de entrada para hotbar (hotbar_1, hotbar_2, hotbar_3)

### hud.gd

- ✅ Usa `get_selected_item()` em vez de `get_current_item()`
- ✅ Usa `get_display_name()` em vez de `.display_name`
- ✅ Null check para hotbar e item

### MultiplayerGameManager.gd

- ✅ Corrigido: `str` → `String` (type error)

### hotbar.tres

- ✅ Atualizado para apontar a `res://inventory/Hotbar.gd`

### export_presets.cfg

- ✅ Criado para compilação Linux/X11 x86_64

### mp_player_ai.tres

- ✅ Fixado de placeholder inválido para recurso mínimo válido

## Compatibilidade Backward

Criados redirects na pasta `gameplay/` para permitir que .tres files antigos funcionem:

- `gameplay/core/State.gd`, `Compose.gd`, `ItemData.gd`, `Hotbar.gd`
- `gameplay/resources/AttackData.gd`, `MoveData.gd`, `CharacterStats.gd`

Todos os .tres files (compose, states, items) continuam carregando sem errors.

## Integração com Player

O Player agora:

1. Cria hotbar automaticamente se não existir
2. Conecta sinais entre hotbar e ASComponent
3. Muda tags de arma (`weapon.`) quando hotbar selection muda
4. Suporta input para seleção de hotbar (hotbar_1, hotbar_2, hotbar_3)

## Próximos Passos

1. **Criar itens de exemplo**: .tres files em `demo/inventory/resources/items/`
2. **Testes de gameplay**: Verificar se itens podem ser coletados/equipados
3. **UI de inventário**: Mostrar grid de 20 slots
4. **Animations**: Sincronizar com AttackData/MoveData

## Status de Compilação

✅ Todos pre-commit hooks passando
✅ Sem syntax errors
✅ Sem referências quebradas a gameplay/
✅ Pronto para compilação da demo
