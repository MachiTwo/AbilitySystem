# 🛠️ Referência da API (v0.1.0 Stable)

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
    - `TAG_TYPE_CONDITIONAL` (1): Tags de estado/condição (state.stunned, state.dead).

- **Métodos:**
  - `register_tag(tag, type, owner_id)`: Registra uma nova tag globalmente.
  - `unregister_tag(tag)`: Remove o registro de uma tag.
  - `is_tag_registered(tag) -> bool`: Verifica registro.
  - `tag_matches(tag, match_against, exact) -> bool` (estático): Comparação hierárquica.
  - `register_resource_name(name, owner_id) -> bool`: Valida unicidade.

---

### `ASComponent` (ASC)

O hub central de lógica para qualquer ator.

#### Habilidades (Abilities)

| Método                                      | Retorno | Descrição                                         |
| :------------------------------------------ | :------ | :------------------------------------------------ |
| `try_activate_ability_by_tag(tag)`          | `bool`  | Tenta ativar via tag. Retorna `true` se iniciada. |
| `try_activate_ability_by_resource(ability)` | `bool`  | Tenta ativar via recurso.                         |
| `cancel_ability_by_tag(tag)`                | `void`  | Interrompe execuções ativas da habilidade.        |
| `unlock_ability_by_tag(tag)`                | `void`  | Libera o uso de uma habilidade do catálogo.       |
| `lock_ability_by_tag(tag)`                  | `void`  | Bloqueia o uso de uma habilidade.                 |
| `add_tag(tag)`                              | `void`  | Adiciona tag e dispara **Ability Triggers**.      |
| `remove_tag(tag)`                           | `void`  | Remove tag e dispara **Ability Triggers**.        |

#### Efeitos (Effects)

| Método                            | Retorno | Descrição                               |
| :-------------------------------- | :------ | :-------------------------------------- |
| `apply_effect_spec_to_self(spec)` | `void`  | Aplica um payload diretamente ao ator.  |
| `has_active_effect_by_tag(tag)`   | `bool`  | Verifica presença de modificador ativo. |
| `remove_effect_by_tag(tag)`       | `void`  | Remove o efeito e limpa modificadores.  |

#### Configuração

| Método                       | Retorno | Descrição                                                          |
| :--------------------------- | :------ | :----------------------------------------------------------------- |
| `apply_container(container)` | `void`  | Inicializa o ASC com o blueprint (atribs/habilidades).             |
| `register_node(alias, node)` | `void`  | Registra nós internos para lookup por Cues (ex: "Muzzle", "Head"). |

---

### `ASDelivery` (Novo na v0.1.0)

Especializado em transportar payloads de efeito entre componentes. Ideal para Projéteis e Áreas de Efeito (AoE).

- **Uso Comum:** Anexar ao script do projétil para lidar com a colisão.
- **Métodos:**
  - `deliver(target_asc)`: Injeta todos os efeitos registrados no ASC alvo.
  - `add_effect(effect)`: Adiciona um recurso de efeito ao payload de entrega.
  - `set_source_component(asc)`: Define quem é o autor do disparo (para cálculo de atributos na fonte).

---

## 2. Recursos (Blueprints)

### `ASAbility`

- **Triggers (Gatilhos):** Ativação reativa baseada em Tags.
  - `TRIGGER_ON_TAG_ADDED`: Ativa quando a tag entra no ASC.
  - `TRIGGER_ON_TAG_REMOVED`: Ativa quando a tag sai (ex: habilidade de morte ao perder a tag `state.alive`).
- **Cooldowns & Custos:** Gerenciados automaticamente pelo Core.

---

## 3. Objetos de Runtime (Specs)

### `ASAbilitySpec` / `ASEffectSpec`

Representam instâncias ativas. Note que o parâmetro `target_node` foi removido para priorizar o uso de **Node Registration** no ASC e o sistema de **ASDelivery**.

---

## 4. Integração LimboAI

O `ASComponent` expõe gatilhos simplificados para `BTAction`. No Behavior Tree, utilize tags para gerenciar o fluxo:

- `BTCheckTag`: Verifica estado.
- `BTActivateAbility`: Dispara a ação via tag.
