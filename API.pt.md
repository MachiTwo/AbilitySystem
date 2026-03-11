# 🛠️ Referência da API

> [!TIP]
> **Leia isto em outros idiomas / Read this in other languages:**
> [**Português**](API.pt.md) | [**English**](API.md)

---

## 1. Sistemas Principais

### `AbilitySystem` (Singleton Global)

Autoridade central para o registro de Tags e nomes de recursos únicos.

- **Enums:**
 	- `TagType`:
  		- `TAG_TYPE_NAME` (0): Tags de identidade (Ability, Effect, Cue, Attribute).
  		- `TAG_TYPE_CONDITIONAL` (1): Tags de estado/condição (state.stunned, state.dead, cooldown.attack).

- **Métodos:**
 	- `register_tag(tag, type, owner_id)`: Registra uma nova tag globalmente com uma classificação.
 	- `unregister_tag(tag)`: Remove o registro de uma tag.
 	- `is_tag_registered(tag) -> bool`: Verifica se uma tag existe no registro central.
 	- `get_tag_type(tag) -> TagType`: Retorna a classificação de uma tag.
 	- `get_registered_tags() -> StringName[]`: Lista todas as tags registradas.
 	- `get_registered_tags_of_type(type) -> StringName[]`: Lista as tags de uma classificação específica.
 	- `get_tag_owner(tag) -> int`: Retorna o ID da instância do recurso que registrou esta tag.
 	- `register_resource_name(name, owner_id) -> bool`: Valida e registra unicidade de nomes de recursos.
 	- `unregister_resource_name(name)`: Remove um nome de recurso do registro global.
 	- `get_resource_name_owner(name) -> int`: Retorna o ID do dono de um nome específico.
 	- `tag_matches(tag, match_against, exact) -> bool` (estático): Utilitário de comparação hierárquica de tags.
- **Sinais:**
 	- `tags_changed`: Emitido quando a lista de tags global é alterada.

---

### `AbilitySystemComponent` (ASC)

O coração do sistema. Deve ser filho direto de um `CharacterBody2D` ou `CharacterBody3D`.

#### Habilidades

| Método | Retorno | Descrição |
| :--- | :--- | :--- |
| `give_ability_by_resource(ability, level)` | `void` | Concede uma habilidade ao ator via recurso. |
| `give_ability_by_tag(tag, level)` | `void` | Concede uma habilidade do catálogo via tag. |
| `remove_ability_by_resource(ability)` | `void` | Revoga uma habilidade via recurso. |
| `remove_ability_by_tag(tag)` | `void` | Revoga todas as habilidades que correspondam à tag. |
| `remove_granted_ability_spec(spec)` | `void` | Remove uma instância específica de habilidade. |
| `can_activate_ability_by_tag(tag)` | `bool` | Verifica se uma habilidade pode ser ativada **sem** ativá-la. Emite `ability_failed` se não puder. |
| `can_activate_ability_by_resource(ability)` | `bool` | Idem, por recurso. |
| `try_activate_ability_by_tag(tag)` | `bool` | Tenta ativar uma habilidade. Retorna `true` se bem-sucedido. |
| `try_activate_ability_by_resource(ability)` | `bool` | Idem, por recurso. |
| `cancel_ability_by_tag(tag)` | `void` | Cancela todas as habilidades ativas com a tag correspondente. |
| `cancel_ability_by_resource(ability)` | `void` | Cancela a habilidade ativa via recurso. |
| `cancel_all_abilities()` | `void` | Força o cancelamento de todas as habilidades ativas. |
| `get_granted_abilities_debug()` | `AbilitySystemAbilitySpec[]` | Retorna todas as habilidades concedidas (para debug). |

#### Efeitos

| Método | Retorno | Descrição |
| :--- | :--- | :--- |
| `make_outgoing_spec(effect, level, target_node)` | `AbilitySystemEffectSpec` | Cria uma instância de efeito validada a partir de um recurso. |
| `apply_effect_spec_to_self(spec)` | `void` | Aplica um spec ao próprio ator. |
| `apply_effect_spec_to_target(spec, target)` | `void` | Aplica um spec ao ASC de outro ator. |
| `apply_effect_by_resource(effect, level, target_node)` | `void` | Aplica diretamente, sem verificar requisitos. |
| `apply_effect_by_tag(tag, level, target_node)` | `void` | Idem, buscando o efeito no catálogo pela tag. |
| `can_activate_effect_by_resource(effect)` | `bool` | Verifica requisitos de ativação de um efeito. |
| `can_activate_effect_by_tag(tag)` | `bool` | Idem, por tag. |
| `try_activate_effect_by_resource(effect, level, target_node)` | `bool` | Aplica se os requisitos forem atendidos. |
| `try_activate_effect_by_tag(tag, level, target_node)` | `bool` | Idem, por tag. |
| `cancel_effect_by_tag(tag)` | `void` | Remove todos os efeitos ativos com a tag. |
| `cancel_effect_by_resource(effect)` | `void` | Remove todos os efeitos ativos do recurso. |
| `remove_active_effect(spec)` | `void` | Remove explicitamente uma instância de efeito ativo. |
| `remove_effect_by_tag(tag)` | `void` | Remove todos os efeitos ativos com a tag correspondente. |
| `remove_effect_by_resource(effect)` | `void` | Remove todas as instâncias ativas do efeito. |
| `has_active_effect_by_tag(tag)` | `bool` | Verifica se há algum efeito ativo com a tag. |
| `has_active_effect_by_resource(effect)` | `bool` | Verifica se há alguma instância ativa do efeito. |
| `get_active_effects_debug()` | `AbilitySystemEffectSpec[]` | Retorna todos os efeitos ativos (para debug). |

#### Atributos

| Método | Retorno | Descrição |
| :--- | :--- | :--- |
| `add_attribute_set(set)` | `void` | Adiciona um AttributeSet único ao componente (cria uma cópia interna). |
| `get_attribute_value_by_tag(tag)` | `float` | Retorna o valor **atual** (base + modificadores). |
| `get_attribute_value_by_resource(attribute)` | `float` | Idem, por recurso. |
| `get_attribute_base_value_by_tag(tag)` | `float` | Retorna o valor **base** sem modificadores. |
| `get_attribute_base_value_by_resource(attribute)` | `float` | Idem, por recurso. |
| `set_attribute_base_value_by_tag(tag, value)` | `void` | Define o valor base diretamente (mudança permanente). |
| `set_attribute_base_value_by_resource(attribute, value)` | `void` | Idem, por recurso. |
| `has_attribute_by_tag(tag)` | `bool` | Verifica se o componente possui o atributo. |
| `has_attribute_by_resource(attribute)` | `bool` | Idem, por recurso. |

#### Tags

| Método | Retorno | Descrição |
| :--- | :--- | :--- |
| `add_tag(tag)` | `void` | Adiciona uma tag ao ator. |
| `remove_tag(tag)` | `void` | Remove uma tag do ator. |
| `has_tag(tag)` | `bool` | Verifica presença de tag (com suporte hierárquico). |
| `get_owned_tags()` | `AbilitySystemTagSpec` | Retorna o container de tags de alta performance. |

#### Cooldowns

| Método | Retorno | Descrição |
| :--- | :--- | :--- |
| `start_cooldown(ability_tag, duration, tags)` | `void` | Inicia um cooldown manualmente com tags associadas. |
| `is_on_cooldown(ability_tag)` | `bool` | Verifica se uma tag está em cooldown. |
| `get_cooldown_remaining(ability_tag)` | `float` | Retorna o tempo restante de um cooldown. |

#### Cues (Feedback Visual/Sonoro)

| Método | Retorno | Descrição |
| :--- | :--- | :--- |
| `register_cue_resource(cue)` | `void` | Registra um recurso de cue para lookup em runtime. |
| `unregister_cue_resource(tag)` | `void` | Remove o registro de um cue pela sua tag. |
| `get_cue_resource(tag)` | `AbilitySystemCue` | Retorna o cue associado à tag. |
| `can_activate_cue_by_tag(tag)` | `bool` | Verifica requisitos de ativação de um cue por tag. |
| `can_activate_cue_by_resource(cue)` | `bool` | Idem, por recurso. |
| `try_activate_cue_by_tag(tag, data, target_node)` | `bool` | Ativa um cue se os requisitos forem atendidos. |
| `try_activate_cue_by_resource(cue, data, target_node)` | `bool` | Idem, por recurso. |
| `cancel_cue_by_tag(tag)` | `void` | Cancela os cues ativos com a tag. |
| `cancel_cue_by_resource(cue)` | `void` | Cancela o cue ativo. |
| `play_montage(name, target)` | `void` | Toca uma animação no nó alvo ou no nó padrão. |
| `is_montage_playing(name, target)` | `bool` | Verifica se uma animação está tocando. |
| `play_sound(sound, target)` | `void` | Toca um áudio no nó alvo ou no nó padrão. |

#### Configuração & Nós

| Método | Retorno | Descrição |
| :--- | :--- | :--- |
| `apply_ability_container(container, level)` | `void` | Inicializa o componente com um blueprint completo (AbilityContainer). |
| `set_animation_player(node)` / `get_animation_player()` | `void / Node` | Define/obtém o nó padrão para animações (AnimationPlayer ou AnimatedSprite). |
| `set_audio_player(node)` / `get_audio_player()` | `void / Node` | Define/obtém o nó padrão para sons. |
| `register_node(name, node)` | `void` | Registra um nó com um alias para uso por cues e efeitos. |
| `unregister_node(name)` | `void` | Remove o registro de um alias de nó. |
| `get_node_ptr(name)` | `Node` | Retorna um nó registrado pelo alias. |
| `register_task(task)` | `void` | Registra uma tarefa assíncrona ativa. |
| `tick(delta)` | `void` | Avança a simulação interna manualmente. Chamado automaticamente pelo `_process`. |
| `calculate_modifier_magnitude(spec, index)` | `float` | Calcula a magnitude de um modificador num spec. |

#### Método Virtual

```gdscript
func _on_calculate_custom_magnitude(spec: AbilitySystemEffectSpec, index: int) -> float:
    pass
```

- `index = -1`: Duração do efeito.
- `index = -2`: Cooldown nativo da habilidade.
- `index = -3` e abaixo: Custos nativos da habilidade (um por custo).

#### Sinais

| Sinal | Parâmetros | Emitido quando |
| :--- | :--- | :--- |
| `ability_activated(spec)` | `AbilitySystemAbilitySpec` | Uma habilidade é ativada com sucesso. |
| `ability_ended(spec, was_cancelled)` | `AbilitySystemAbilitySpec, bool` | Uma habilidade termina ou é cancelada. |
| `ability_failed(ability_name, reason)` | `StringName, String` | Uma habilidade falha ao ativar, com o motivo. |
| `attribute_changed(name, old, new)` | `StringName, float, float` | O valor de um atributo muda. |
| `tag_changed(tag_name, is_present)` | `StringName, bool` | Uma tag é adicionada ou removida. |
| `tag_event_received(event_tag, data)` | `StringName, Dictionary` | Um evento de tag específico é disparado. |
| `effect_applied(spec)` | `AbilitySystemEffectSpec` | Um efeito é aplicado com sucesso. |
| `effect_removed(spec)` | `AbilitySystemEffectSpec` | Um efeito ativo é removido. |
| `effect_failed(effect_name, reason)` | `StringName, String` | Um efeito falha ao ser aplicado. |
| `cue_failed(cue_name, reason)` | `StringName, String` | Um cue falha ao ser ativado. |
| `cooldown_started(ability_tag, duration)` | `StringName, float` | Um cooldown começa. |
| `cooldown_ended(ability_tag)` | `StringName` | Um cooldown chega a zero. |

---

## 2. Recursos (Blueprints de Dados)

### `AbilitySystemAbility`

Define a lógica e os dados de uma ação específica.

- **Propriedades:** `ability_name`, `ability_tag`, `activation_required_tags`, `activation_blocked_tags`, `activation_owned_tags`, `effect`, `costs`, `cooldown_duration`, `cooldown_tags`, `cooldown_use_custom`, `costs_use_custom`, `cues`.
- **Métodos Virtuais (sobrescrever em GDScript):**
 	- `_on_can_activate_ability(owner, spec) -> bool`: Requisitos customizados de ativação.
 	- `_on_activate_ability(owner, spec)`: Lógica principal da habilidade.
 	- `_on_end_ability(owner, spec)`: Limpeza ao encerrar.
- **Métodos de Ciclo de Vida:**
 	- `can_activate_ability(owner, spec) -> bool`: Valida tags, custos e cooldown.
 	- `activate_ability(owner, spec, target_node)`: Inicia a execução (aplica custos e cooldown automaticamente).
 	- `end_ability(owner, spec)`: Encerra a execução (limpa `activation_owned_tags`).
 	- `can_afford_costs(owner, spec) -> bool`: Verifica se o ator tem recursos para os custos.
 	- `apply_costs(owner, spec)`: Aplica os custos ao ator.
 	- `add_cost(attribute, amount)`: Adiciona um custo nativo em runtime.
 	- `remove_cost(attribute) -> bool`: Remove um custo nativo em runtime.

> [!NOTE]
> As `activation_owned_tags` são automaticamente adicionadas ao ASC quando a habilidade ativa e removidas quando ela termina. Use-as para representar o estado ativo da habilidade (ex: `ability.skill.walk`).

---

### `AbilitySystemEffect`

Define um pacote de modificadores de atributos e tags associadas.

- **Políticas de Duração** (`DurationPolicy`):
 	- `POLICY_INSTANT` (0): Aplicado uma vez e removido imediatamente.
 	- `POLICY_DURATION` (1): Dura um tempo definido.
 	- `POLICY_INFINITE` (2): Dura indefinidamente até ser removido.
- **Políticas de Stacking** (`StackingPolicy`):
 	- `STACK_NEW_INSTANCE` (0): Cada aplicação é independente.
 	- `STACK_OVERRIDE` (1): Nova aplicação substitui a antiga (reinicia o timer).
 	- `STACK_INTENSITY` (2): Aumenta a magnitude (soma stacks).
 	- `STACK_DURATION` (3): Estende a duração restante.
- **Operações de Modificador** (`ModifierOp`): `OP_ADD`, `OP_MULTIPLY`, `OP_DIVIDE`, `OP_OVERRIDE`.
- **Propriedades chave:** `effect_name`, `effect_tag`, `duration_policy`, `duration_magnitude`, `granted_tags`, `removed_tags`, `activation_required_tags`, `activation_blocked_tags`, `blocked_tags`, `use_custom_duration`, `period`, `execute_periodic_tick_on_application`, `stacking_policy`, `cues`.
- **Método:** `add_modifier(attribute, operation, magnitude, use_custom_magnitude)`.

---

### `AbilitySystemAttributeSet`

Container de atributos de um ator. Uma instância única é criada por ator no `apply_ability_container`.

- **Propriedades:** `attribute_definitions`, `granted_abilities`.
- **Métodos:** `add_attribute_definition(attribute)`, `remove_attribute_definition(name)`, `get_attribute_definition(name)`, `get_attribute_list()`, `add_modifier(name, value, type)`, `remove_modifier(name, value, type)`, `get_attribute_current_value(name)`, `get_attribute_base_value(name)`, `set_attribute_base_value(name, value)`, `set_attribute_current_value(name, value)`, `has_attribute(name)`, `reset_current_values()`.
- **Sinais:** `attribute_changed(name, old, new)`, `attribute_pre_change(name, old, new)`.

---

### `AbilitySystemAbilityContainer` (Arquétipo)

Blueprint completo de um arquétipo de ator. Atribuído à propriedade `ability_container` do ASC.

- **Propriedades:** `abilities`, `attribute_set`, `effects`, `cues`.

> [!IMPORTANT]
> Todos os efeitos e cues que o ASC pode emitir **devem** estar registrados no seu `AbilityContainer`. O sistema valida isso em `make_outgoing_spec`.

---

## 3. Objetos de Runtime (Specs)

### `AbilitySystemAbilitySpec`

Instância de uma habilidade concedida. Armazena estado de runtime.

| Método | Retorno | Descrição |
| :--- | :--- | :--- |
| `get_ability()` | `AbilitySystemAbility` | Retorna o recurso de habilidade. |
| `get_level()` | `int` | Retorna o nível desta instância. |
| `get_is_active()` | `bool` | Retorna `true` se a habilidade está em execução. |
| `get_owner()` | `AbilitySystemComponent` | Retorna o ASC dono desta spec. |
| `get_cooldown_duration()` | `float` | Retorna a duração base do cooldown. |
| `get_cooldown_remaining()` | `float` | Retorna o tempo restante do cooldown no ASC dono. |
| `is_on_cooldown()` | `bool` | Verifica se a tag da habilidade está em cooldown. |
| `get_cost_amount(attribute)` | `float` | Retorna o custo para um atributo específico. |

---

### `AbilitySystemEffectSpec`

Instância de um efeito ativo. Armazena duração calculada, stacks e magnitudes.

| Método | Retorno | Descrição |
| :--- | :--- | :--- |
| `get_effect()` | `AbilitySystemEffect` | Retorna o recurso de efeito. |
| `get_duration_remaining()` | `float` | Tempo restante antes do efeito acabar. |
| `get_total_duration()` | `float` | Duração inicial calculada. |
| `get_stack_count()` | `int` | Número atual de stacks. |
| `get_level()` | `float` | Nível de poder desta instância. |
| `get_magnitude(name)` | `float` | Valor calculado do modificador para um atributo. |
| `get_source_component()` | `AbilitySystemComponent` | O ASC que aplicou este efeito. |
| `get_target_component()` | `AbilitySystemComponent` | O ASC destino onde o efeito está aplicado. |
| `get_source_attribute_value(attribute)` | `float` | Valor de um atributo na fonte (para cálculos relativos). |
| `get_target_attribute_value(attribute)` | `float` | Valor de um atributo no alvo. |
| `get_period_timer()` | `float` | Tempo restante até o próximo tick periódico. |
| `get_hit_position()` | `Variant` | Posição de impacto (Vector2 ou Vector3). |

---

### `AbilitySystemTagSpec`

Container de alta performance para tags de estado do ator. Tags são referenciadas por contagem — uma tag só é removida quando sua contagem chega a zero.

| Método | Retorno | Descrição |
| :--- | :--- | :--- |
| `add_tag(tag)` | `bool` | Adiciona a tag (ou incrementa sua contagem). Retorna `true` na primeira referência. |
| `remove_tag(tag)` | `bool` | Decrementa a contagem. Retorna `true` quando a tag é de fato removida. |
| `has_tag(tag, exact)` | `bool` | Verifica presença (hierárquica por padrão). |
| `has_all_tags(tags, exact)` | `bool` | Verifica se todas as tags estão presentes. |
| `has_any_tags(tags, exact)` | `bool` | Verifica se alguma das tags está presente. |
| `get_all_tags()` | `StringName[]` | Retorna todas as tags do container. |
| `clear()` | `void` | Remove todas as tags. |

---

### `AbilitySystemCueSpec` (Contexto de Cue)

Pacote de contexto passado aos métodos virtuais de cue.

- **Propriedades:** `cue`, `effect_spec`, `extra_data`, `hit_position`, `magnitude`, `source_asc`, `target_asc`, `target_node`.
- **Métodos:** `get_level()`, `set_level(level)`.

---

## 4. Extensibilidade

### `AbilitySystemTask` (Lógica Assíncrona)

Usado internamente por habilidades para gerenciar operações com tempo (esperas, animações).

- **Métodos Estáticos:** `wait_delay(owner, delay) -> Task`, `play_montage(owner, animation_name) -> Task`.

---

### `AbilitySystemCue` (Base de Feedback)

Recurso base para feedbacks audiovisuais. Use `AbilitySystemCueAnimation` ou `AbilitySystemCueAudio` para casos comuns. Extenda em GDScript para comportamento totalmente customizado.

- **Propriedades:** `cue_name`, `cue_tag`, `event_type` (`ON_EXECUTE`, `ON_ACTIVE`, `ON_REMOVE`), `node_name`.
- **Métodos Virtuais:** `_on_execute(spec)`, `_on_active(spec)`, `_on_remove(spec)`.

---

## 5. Ferramentas do Editor (API Interna)

> [!IMPORTANT]
> Estas ferramentas requerem que o plugin esteja ativado em **Configurações do Projeto > Plugins**.

| Ferramenta | Propósito |
| :--- | :--- |
| **`AbilitySystemTagsPanel`** | Painel no Project Settings para gerenciar todas as tags hierárquicas (SSOT). |
| **`AbilitySystemInspectorPlugin`** | Registra os editores customizados no Inspetor do Godot. |
| **`AbilitySystemEditorPropertyName`** | Campo de nome com validação de unicidade em tempo real. |
| **`AbilitySystemEditorPropertySelector`** | Seletor em árvore para propriedades de múltiplas tags. |
| **`AbilitySystemEditorPropertyTagSelector`** | Dropdown para seleção de tag única. |
