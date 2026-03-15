# 🛠️ Referência da API (v0.1.0 Stable)

> [!TIP]
> **Leia isto em outros idiomas / Read this in other languages:**
> [**Português**](API.pt.md) | [**English**](API.md)

---

## 1. Sistemas Principais

### `AbilitySystem` (Singleton Global)

A autoridade central para o sistema de Tags e registro de nomes de recursos.

- **Constantes (TagType):**
  - `TAG_TYPE_NAME` (0): Tags de identidade para recursos permanentes.
  - `TAG_TYPE_CONDITIONAL` (1): Tags de estado dinâmico (ex: `state.stunned`).

- **Métodos:**
  - `register_tag(tag, type=0, owner_id=0)`: Registra uma tag globalmente.
  - `unregister_tag(tag)`: Remove uma tag.
  - `rename_tag(old, new)`: Renomeia uma tag e todos os seus filhos.
  - `remove_tag_branch(tag)`: Remove recursivamente uma tag e sua sub-hierarquia.
  - `tag_matches(tag, match_against, exact=false)` (estático): Lógica de comparação hierárquica.
  - `get_registered_tags()`: Retorna um array com todas as tags globais.

---

### `ASComponent` (ASC)

O centro lógico de qualquer ator. Gerencia atributos, habilidades, efeitos e tags.

#### 🔖 Tags & Estado

- `add_tag(tag)`: Adiciona uma tag e dispara reavaliações.
- `remove_tag(tag)`: Remove uma tag.
- `has_tag(tag)`: Verifica a presença de uma tag (suporta hierarquia).
- `get_tags()`: Retorna todas as tags atualmente ativas.

#### ⚔️ Habilidades

- `try_activate_ability_by_tag(tag)`: Tenta a ativação via tag.
- `try_activate_ability_by_resource(ability)`: Tenta a ativação via recurso.
- `cancel_ability_by_tag(tag)`: Aborta instâncias ativas que coincidam com a tag.
- `unlock_ability_by_tag(tag)`: Torna uma habilidade disponível para o ator.
- `is_ability_active(tag)`: Verifica se uma habilidade está em execução.
- `get_active_abilities()`: Retorna instâncias atuais de `ASAbilitySpec`.

#### 📊 Atributos

- `get_attribute_value_by_tag(tag)`: Obtém o valor calculado atual.
- `get_attribute_base_value_by_tag(tag)`: Obtém o valor base não modificado.
- `set_attribute_base_value_by_tag(tag, value)`: Atualiza o valor base.
- `has_attribute_by_tag(tag)`: Verifica se o atributo existe neste ator.

#### ✨ Efeitos

- `apply_effect_spec_to_self(spec)`: Injeta diretamente um payload de efeito.
- `has_active_effect_by_tag(tag)`: Verifica se um efeito específico está ativo.
- `remove_effect_by_tag(tag)`: Remove instâncias do efeito.

#### 🎭 Cues & Notificações

- `register_node(alias, node)`: Registra um nó para busca de cues (ex: "Muzzle").
- `try_activate_cue_by_tag(tag, data={})`: Dispara feedback visual/sonoro.
- `set_animation_player(node)`: Registra o controlador principal de animação.

---

## 2. Recursos (Blueprints)

### `ASAbility`

Configuração para ações com custos, cooldowns e requisitos.

- **Políticas de Duração:** `INSTANT`, `DURATION`, `INFINITE`.
- **Gatilhos (Triggers):** `ON_TAG_ADDED`, `ON_TAG_REMOVED`.
- **Propriedades Chave:** `costs`, `requirements`, `cooldown_duration`, `activation_owned_tags`.

### `ASEffect`

Blueprints para modificações de atributos e injeção de tags.

- **Operações de Modificador:** `ADD`, `MULTIPLY`, `DIVIDE`, `OVERRIDE`.
- **Empilhamento (Stacking):** `NEW_INSTANCE`, `OVERRIDE`, `INTENSITY`, `DURATION`.
- **Periódicos:** Suporta `period` for tick-based execution (DoT/HoT).

### `ASContainer`

Um banco de dados de arquétipo (AttributeSet + Habilidades Padrão + Efeitos Iniciais). Use `asc.apply_container(resource)` para inicializar um ator.

### `ASPackage`

Um pacote portátil de Efeitos e Cues. Ideal para itens ou lógica de múltiplos passos.

---

## 3. Sistemas Especializados

### `ASDelivery`

Gerencia o transporte de payloads para projéteis, armadilhas e AoEs.

- `add_effect(effect)` / `add_cue(cue)`: Prepara o payload.
- `deliver(target_asc)`: Injeta o payload em um ASC de destino.

---

## 4. Objetos de Runtime

### `ASAbilitySpec` / `ASEffectSpec`

Instâncias ativas de seus respectivos recursos. Mantêm o estado variável (duração restante, nível, etc.).

---

## 5. Integração com LimboAI

O `ASComponent` é nativamente compatível com LimboAI via:

- **Triggers:** Reaja a mudanças de tags na Árvore de Comportamento.
- **Nós BT:** (Recomendado) Use `BTCheckTag` e `BTActivateAbility` para lógica de IA baseada em estados.

---

> [!NOTE]
> Para a referência completa das classes em C++, consulte a ajuda interna da engine ou o diretório `doc/source/classes`.
