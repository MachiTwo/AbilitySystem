# Ability System

> [!TIP]
> **Leia isto em outros idiomas / Read this in other languages:**
> [**Português**](release_note.pt.md) | [**English**](release_note.md)

## 1. Contexto e Filosofia

O **Ability System** para Godot Engine é um framework nativo de alta performance projetado para lidar com lógica de combate, habilidades e atributos de forma modular e baseada em dados.

Diferente de implementações em script puro, o Ability System foca em:

- **Performance Bruta:** Cálculos de modificadores e processamento de tags em C++.
- **Data-Driven:** Habilidades e efeitos são `Resource`, permitindo que designers criem mecânicas sem tocar em código.
- **Desacoplamento:** A lógica de "como um dano é calculado" é separada de "quem causa o dano".

## 2. Arquitetura Core (C++)

O sistema está organizado em subpastas em `modules/ability_system/` para melhor manutenibilidade:

- `core/`: Singleton central, Tags e Specs.
- `resources/`: Definições de Atributos, Habilidades e Efeitos.
- `scene/`: Componentes de Cena.

### 2.0 AbilitySystem (Singleton)

O ponto central de verdade do sistema. Gerenciado como um Singleton da Engine [Object].

- **Global Tag Registry:** Armazena todas as tags registradas no projeto.
- **Project Settings:** Integrado às configurações do projeto para persistência de definições de tags.

### 2.1 AbilitySystemComponent (ASC)

O "cérebro" do Ator. Deve ser anexado a um `CharacterBody2D/3D` [Node].

- **API de Registro (Setup Manual):**
  - `set_animation_player(Node)`: Define o slot padrão para animações (aceita `AnimationPlayer` ou `AnimatedSprite`).
  - `set_audio_player(Node)`: Define le slot padrão para áudio (aceita `AudioStreamPlayer`, `2D` ou `3D`).
  - `register_node(StringName name, Node)`: Registra um nó nomeado para Cues específicos.
- **API de Feedback (Execução):**
  - `play_montage(StringName anim, Node target)`: Toca animação. Se `target` nulo, usa o slot padrão.
  - `play_sound(AudioStream sound, Node target)`: Toca áudio. Se `target` nulo, usa o slot padrão.
- **API Principal (Execução):**
  - `give_ability(AbilitySystemAbility ability)`: Registra uma nova habilidade.
  - `try_activate_ability(Variant ability_or_tag)`: Tenta executar a habilidade passada (aceita recurso, tag ou nome).
  - `cancel_ability(Variant ability_or_tag)`: Encerra uma habilidade ativa.
  - `apply_effect_spec_to_self(AbilitySystemEffectSpec spec)`: Aplica modificadores.
- **Suporte de Posição:** Todos os parâmetros de posição (impactos, cues) agora usam `Variant`, suportando tanto `Vector2` quanto `Vector3`.
- **Signals Reativos:**
  - `attribute_changed(name, old, new)`: Para UI e Feedback.
  - `tag_changed(name, present)`: Para mudanças de estado visual.

> [!NOTE]
> Os métodos `play_montage` e `play_sound` são públicos e podem ser usados via script, mas o fluxo recomendado é o **Data-Driven**, onde as Habilidades e Efeitos disparam esses métodos internamente através de Cues.

### 2.2 AbilitySystemAbilityContainer (Resource)

Define o **contrato de permissão** (arquétipo) de uma entidade, catalogando tudo o que ela é capaz de possuir ou realizar.

- **Propósito:** Atua como um "blueprint" ou "template" fixo para raças, classes ou tipos de inimigos. Ele define o escopo de possibilidades, permitindo que o jogo gerencie o desbloqueio progressivo (via nível ou classe) de forma segura.
- **Conteúdo (Catálogo):**
  - `abilities`: Lista de habilidades que a entidade **pode** aprender ou equipar (catálogo de acesso).
  - `effects`: Catálogo de efeitos, passivas e **instâncias de ataque** permitidas (ex: veneno, escudos, buffs), que o arquétipo está autorizado a carregar ou instanciar.
  - `attributes`: Atributos permitidos e suas definições para este arquétipo.
  - `tags`: Tags de identidade e permissão inerentes ao arquétipo.
  - `cues`: Mapeamento de feedbacks audiovisuais disponíveis para o arquétipo.

### 2.3 AbilitySystemAttribute

Define um atributo individual com schema, limites e validação.

- **Propósito:** Resource de definição para estatísticas (HP, Mana, Stamina).
- **Propriedades:**
  - `name`: Identificador único do atributo.
  - `base_value`: Valor base inicial.
  - `min_value`/`max_value`: Limites para validação automática.
- **Validação:** Métodos `is_valid_value()` e `clamp_value()` para garantir consistência.
- **Uso:** Configurável no Editor, reutilizável entre múltiplos personagens.

### 2.4 AbilitySystemAttributeSet

Container que gerencia coleções de AbilitySystemAttribute Resources.

- **Modularidade:** Suporta múltiplos sets por AbilitySystemComponent.
- **Validação Obrigatória:** Exige que atributos sejam definidos como Resources antes do uso.
- **API Resource-based:**
  - `add_attribute_definition(AbilitySystemAttribute)`: Registra definição.
  - `get_attribute_definition(name)`: Obtém Resource de definição.
  - `set_attribute_base_value(name, value)`: Manipula com validação automática.
- **Runtime Performance:** Mantém HashMap interno para valores (base/current) com validação via Resource.
- **Sinais:** `attribute_changed`, `attribute_pre_change` para reatividade.

### 2.5 AbilitySystemAbility (Resource)

Define a lógica de "o que acontece" e principalmente "o que esta habilidade é".

- **Identidade vs Comportamento:** Uma habilidade no Ability System se define primeiramente pelo seu `Ability Tag` único, que atua como o seu "Documento de Identidade". O restante dos campos de Tags (os Arrays de Ativação) não ditam _quem_ a Habilidade é, mas sim o seu _comportamento_ mediante o personagem e o cenário.
- **Hierarquia do Polimorfismo:** Por causa do formato das Tags (e.g. `ability.spell.fire.fireball`), a Engine lida facilmente com grupos ou classificadores sem usar um Array de Tags na Identidade. Se o player tentar ativar `ability.spell` no ASC, todas as sub-skills de magia respondem. Se quiser especificidade, chame de forma integral: `ability.spell.fire`.

**Propriedades de Controle de Estado e Condições (As "Activation Tags"):**

- **`ability_tag` (O Registro Único):** Tag principal e singular formadora de identidade e filtragem (`StringName`). Exemplo: `ability.movement.dodge`
- **`activation_owned_tags` (O Buff Temporário):** Array de Tags. Quando a habilidade iniciar, ela carimba todo este grupo no Character e os remove assim que terminar o ciclo de vida. Dita o que seu Personagem "se torna" (ex: durante a Esquiva, ganha _state.immune.damage_ e _state.animation.rolling_).
- **`activation_required_tags` (Os Requisitos):** Array de Tags. A habilidade recusa ativar se tentar ser invocada por um Player que não detenha todas estas tags listadas (Pode exigir _state.combat.advantage_ ou algo nativo como _state.in_air_).
- **`activation_blocked_tags` (O CC/Block):** Array de Tags. A habilidade recusa ativar se o usuário tentar usá-la enquanto estiver sofrendo de qualquer uma destas Tags (As famosas janelas de Stun, Silence e Root - _state.debuff.stun_).

**Outras Estruturas Core:**

- **Fluxo de Vida:** `can_activate` -> `activate_ability` -> `end_ability`.
- **Encapsulamento Audiovisual:** Contém uma lista de `cues` executados automaticamente.
- **Restritores Físicos e Lógicos:**
  - **Custos Nativos:** Array de Dicionários `{attribute: StringName, amount: float}` aplicados diretamente aos atributos do dono.
  - **Cooldown Nativo:** `cooldown_duration` e `cooldown_tags` gerenciados automaticamente pelo ASC.
  - **Lógica Customizada:** Flags `cooldown_use_custom` e `costs_use_custom` para disparar cálculos via script no ASC através de `_on_calculate_custom_magnitude` (Índice -2 para cooldown, -3 e abaixo para custos).
- **Métodos Virtuais:** `_can_activate_ability()`, `_activate_ability()`, `_on_end_ability()`.

### 2.6 AbilitySystemEffect (Resource)

Define as regras e o "pacote" de alteração de um personagem:

- **Lógica de Origem:** Registrado no `AbilitySystemAbilityContainer` de quem **causa** o efeito. O Sniper tem o "efeito tiro" em seu catálogo; o inimigo apenas recebe o impacto do cálculo via código.
- **Variedade:**
  - **Dano Único** (Sniper) vs **Dano Contínuo/DOT** (Veneno, Área de Fogo).
  - **Buffs de Status** (Força, Escudos de Proteção).
- **Políticas de duração:**
  - `INSTANT` — Ex: Um tiro de sniper (dano único registrado na arma/personagem).
  - `DURATION` — Ex: Uma flechada envenenada ou um escudo protetor temporário.
  - `INFINITE` — Ex: Um encantamento de força ou aura permanente.
- **Modificadores:** Operações de `ADD`, `MULTIPLY`, `DIVIDE` e `OVERRIDE`.
- **Costs:** Custos de ativação (Mana, Stamina, etc.).
- **Cooldowns:** Tempo de recarga entre usos.
- **Tags:** `granted_tags`, `removed_tags`, `blocked_tags`.
- **Cues:** `cue_tags` para disparar feedback visual/sonoro.
- **Duração Customizada:** Flag `use_custom_duration` para disparar cálculo via script no ASC através de `_on_calculate_custom_magnitude` (Índice -1).
- **Magnitude Custom:** gerenciada via método virtual `_on_calculate_custom_magnitude` no `AbilitySystemComponent` para qualquer modificador marcado como `use_custom_magnitude`.

### 2.7 AbilitySystemCue (Resource)

Sistema de ativação e sincronização de eventos (animações, sons) disparado por efeitos ou habilidades.

- **Função:** Sistema para ativação e sincronização de eventos de gameplay.
- **Dependência:** Requer classes especializadas (CueAnimation, CueAudio, CueParticles [em planejamento]) para funcionalidade real.
- **Trigger:** Executado quando efeitos são aplicados/removidos ou habilidades ativadas.
- **Tipos:** `ON_EXECUTE` (instantâneo), `ON_ACTIVE` (enquanto ativo), `ON_REMOVE` (ao remover).
- **Arquitetura:** Fornece estrutura base mas não implementa funcionalidade específica.
- **Métodos Virtuais:** `_on_execute()`, `_on_active()`, `_on_remove()` para customização via script.
- **Uso:** Herança para criar cues especializados ou customização via GDScript/C#.

#### 2.7.1 AbilitySystemCueAnimation (Resource)

Especializado em executar animações.

- **Propriedade:** `animation_name` (StringName) - Nome da animação.
- **Propriedade:** `node_name` (StringName) - Nome do nó alvo registrado no ASC (opcional).
- **Execução:** O sistema chama internamente `asc->play_montage(animation_name, target)`. Se `node_name` estiver vazio, usa o `AnimationPlayer` padrão do ASC.

#### 2.7.2 AbilitySystemCueAudio (Resource)

Especializado em reproduzir áudios.

- **Propriedade:** `audio_stream` (Ref AudioStream) - Recurso de áudio.
- **Propriedade:** `node_name` (StringName) - Nome do nó alvo registrado no ASC (opcional).
- **Execução:** O sistema chama internamente `asc->play_sound(audio_stream, target)`. Se `node_name` estiver vazio, usa o `AudioStreamPlayer` padrão do ASC.

**Nota de Fluxo:** Play Montage e Play Sound são a "ponta final" da execução de feedback do sistema. Embora públicos no ASC, seu uso via Cues (Data-Driven) é preferível para manter a lógica desacoplada do código de animação/som.

### 2.8 AbilitySystemTag

Identificador único para estados do jogo [RefCounted].

- **Hierárquico:** Ex: `state.debuff.poison`, `ability.fireball`.
- **Otimizado:** `StringName` puro com armazenamento direto em `HashSet` para máxima performance.
- **Global:** Gerenciado centralmente pelo singleton `AbilitySystem`.
- **Strict-Mode:** No editor, seletores garantem que apenas tags registradas no `AbilitySystemTagsPanel` sejam utilizadas.

### 2.9 AbilitySystemTagSpec

Container de alta performance para operações e consultas de tags [RefCounted].

- **Gerenciamento:** `add_tag()`, `remove_tag()` com reference counting.
- **Consultas:** `has_tag()`, `has_any_tags()`, `has_all_tags()`.
- **Matching:** Suporte a matching hierárquico (`exact` parameter).
- **Performance:** Estrutura otimizada para alta frequência de operações.

### 2.10 AbilitySystemTask

Base para tarefas assíncronas em habilidades [RefCounted].

- **Tipos:** `TASK_WAIT_DELAY`, `TASK_PLAY_MONTAGE`, `TASK_WAIT_EVENT`.
- **Métodos Estáticos:** `wait_delay()`, `play_montage()`.
- **Sinais:** `completed` para coordenação assíncrona.
- **Extensível:** Herança para tarefas customizadas.

## 3. Sistema de Runtime (Specs)

### 3.1 AbilitySystemAbilitySpec

Instância de runtime de uma `AbilitySystemAbility` concedida ao ASC.

- **Dados:** Nível, estado ativo, referência ao Resource.
- **Métodos:** `get_ability()`, `get_is_active()`, `get_level()`.
- **Ciclo:** Gerenciado pelo `AbilitySystemComponent`.

### 3.2 AbilitySystemEffectSpec

Instância de runtime de um `AbilitySystemEffect` aplicado.

- **Dados:** Duração restante, magnitudes dinâmicas, nível.
- **Contexto:** Source/target ASCs, cálculos de magnitude.
- **Métodos:** `get_duration_remaining()`, `set_magnitude()`, `calculate_modifier_magnitude()`.

### 3.3 AbilitySystemCueSpec

Contexto de execução runtime para uma `AbilitySystemCue`.

- **Dados:** Cue resource, effect spec trigger, magnitude calculada.
- **Contexto:** Source/target ASCs, cálculos de magnitude.
- **Métodos:** `get_source_asc()`, `get_magnitude()`, `get_effect_spec()`.
- **Uso:** Passado para callbacks GDVirtual da Cue (`_on_execute`, `_on_active`, `_on_remove`).

**Exemplo (GDScript):**

```gdscript
extends AbilitySystemCue

func _on_execute(spec: AbilitySystemCueSpec):
 var damage = spec.magnitude
 var attacker = spec.source_asc
 # Spawn floating damage number at attacker's position, etc.
 print("Cue triggered! Damage = ", damage)
```

- **Contexto:** Source ASC, target ASC, dados extras.
- **Uso:** Passado para callbacks da Cue com contexto completo.

## 4. Ability System Tags

As **Tags do Ability System** são `StringName` hierárquicos otimizados.

- **Exemplo:** `state.buff.speed`, `state.debuff.stun`.
- **Registro:** Gerenciados pelo singleton `AbilitySystem`.
- **Matching:** Suporte a matching hierárquico via `AbilitySystem.tag_matches()`.
- **Performance:** Otimizados para consultas de alta frequência sem overhead de classes.
- **Workflow:** Registre as tags no Project Settings e use os seletores automáticos nos Resources.

## 5. Ferramentas do Editor

### 5.1 AbilitySystemEditorPlugin

Integração nativa com o **Inspector** da Godot para automatização de UI:

- **Seletores Unificados:** Qualquer propriedade terminada em `_tag` (StringName) ou `_tags` (Array de StringName) recebe automaticamente um seletor dropdown.
- **Sincronização em Real-time:** O seletor se conecta ao sinal `tags_changed` do `AbilitySystem`, garantindo que novos registros no Tag Editor apareçam instantaneamente no Inspetor sem reiniciar a engine.
- **Validação Visual:** Tags selecionadas que não existem mais no registro global são marcadas com um ícone de erro e aviso "(Inexistente)".

### 5.2 AbilitySystemTagsPanel (Tag Registry)

Localizado em **Bottom Panel > Ability System Tags**, é a central de verdade do sistema:

- **Gestão Centralizada:** Adicione ou remova tags globais que serão usadas em todo o projeto.
- **Persistência Segura:** As tags são salvas nas configurações do projeto (`project.godot`), garantindo que a equipe de design tenha o mesmo catálogo validado.
- **Prevenção de Erros:** Evita o uso de "Magic Strings" espalhadas pelo projeto, forçando um workflow baseado em registro prévio.

### 5.3 AbilitySystemEditorPropertySelector (Integração de Propriedades Array)

Componente de UI customizado puro C++ (herdeiro de `EditorProperty`) que intercede no inspetor da Engine substituindo o editor padrão de Arrays para propriedades terminadas em `_tags`.

- **Seleção Dinâmica Multi-Tags:** Oferece um botão que abre um diálogo flutuante. O layout apresenta uma `Tree` listando todas as tags registradas e checkboxes nativos, suportando múltiplas inserções de uma vez.
- **Filtro em Real-time:** Possui um campo de busca dinâmico no topo da lista que mascara e filtra tags (ex: digitar "poison" traz todas as sub-tags associadas para tagging rápido).

### 5.4 AbilitySystemEditorPropertyTagSelector (Integração de Tag Única)

Componente de UI customizado puro C++ (herdeiro de `EditorProperty`) para propriedades de tag única terminadas em `_tag`.

- **Seleção Dropdown:** Fornece um `OptionButton` com todas as tags registradas mais uma opção "(Nenhuma)".
- **Atualizações em Real-time:** Atualiza automaticamente quando tags são adicionadas/removidas do registro global.

### 5.5 AbilitySystemEditorPropertyName (Validação de Nomes)

Editor customizado para propriedades terminadas em `_name` que garante unicidade no projeto.

- **Verificação de Unicidade:** Valida nomes contra recursos existentes do mesmo tipo.
- **Feedback Visual:** Mostra ícones de aviso para nomes duplicados.

### 5.6 AbilitySystemInspectorPlugin (Parser de Propriedades)

Plugin principal que intercepta o parsing de propriedades e direciona para os editores customizados apropriados.

- **Matching de Padrões:** Detecta padrões de propriedade `_tag`, `_tags`, e `_name`.
- **Registro de Editores:** Registra automaticamente o editor customizado correto para cada tipo de propriedade.
- **Resumo Visual Constante:** Mantém sua interface limpa quando recolhido, exibindo diretamente estatísticas de seleção (ex: "4 Tags Selecionadas" ou "(Tags Vazias)").

## 6. Sinais e Eventos

### 6.1 AbilitySystemComponent Signals

- **`ability_activated(spec)`**: Habilidade iniciada.
- **`ability_ended(spec, was_cancelled)`**: Habilidade finalizada.
- **`ability_failed(name, reason)`**: Falha na ativação.
- **`attribute_changed(name, old, new)`**: Mudança de atributo.
- **`tag_changed(name, present)`**: Adição/remoção de tag.
- **`effect_applied(spec)`**: Efeito aplicado.
- **`effect_removed(spec)`**: Efeito removido.
- **`tag_event_received(tag, data)`**: Evento genérico disparado por Cues ou manualmente.
- **`cooldown_started(tag, duration)`**: Início de cooldown.
- **`cooldown_ended(tag)`**: Fim de cooldown.

### 6.2 AbilitySystemAttributeSet Signals

- **`attribute_changed(attribute_name, old_value, new_value)`**: Disparado em qualquer mudança.
- **`attribute_pre_change(attribute_name, old_value, new_value)`**: Antes da mudança (pode ser cancelado).
- **Uso:** Atualizar UI (health bars), triggers de lógica (morte quando HP = 0).

---

## 🎮 Tutorial Completo: Criando um Player de RPG de Ação 2D

Este tutorial demonstra como criar um player 2D completo usando o Ability System Plugin, baseado no nosso projeto demo.

### Passo 1: Criar Atributos do Player

Crie `player_attribute_set.tres`:

```gdscript
extends AbilitySystemAttributeSet

func _init():
 # Atributo Health
 var health = AbilitySystemAttribute.new()
 health.attribute_name = &"Health"
 health.base_value = 100.0
 health.min_value = 0.0
 health.max_value = 100.0
 add_attribute(health)

 # Atributo Stamina (para habilidades)
 var stamina = AbilitySystemAttribute.new()
 stamina.attribute_name = &"Stamina"
 stamina.base_value = 50.0
 stamina.min_value = 0.0
 stamina.max_value = 50.0
 add_attribute(stamina)
```

### Passo 2: Criar Habilidades de Movimento

Crie habilidade `walk.tres`:

```gdscript
extends AbilitySystemAbility

func _init():
 ability_name = "Walk"
 ability_tag = &"ability.skill.walk"

 # Tags aplicadas ao ativar
 activation_owned_tags = [&"ability.skill.walk"]

 # Bloqueia idle enquanto anda
 activation_blocked_tags = [&"ability.skill.idle"]

 # Link para cue de animação
 cues = [preload("res://resources/player/cues/walk_cue.tres")]
```

Crie habilidade `idle.tres`:

```gdscript
extends AbilitySystemAbility

func _init():
 ability_name = "Idle"
 ability_tag = &"ability.skill.idle"

 # Tags aplicadas ao ativar
 activation_owned_tags = [&"ability.skill.idle"]

 # Bloqueia walk enquanto ocioso
 activation_blocked_tags = [&"ability.skill.walk"]

 # Link para cue de animação
 cues = [preload("res://resources/player/cues/idle_cue.tres")]
```

### Passo 3: Criar Habilidades de Combate

Crie habilidade `attack.tres`:

```gdscript
extends AbilitySystemAbility

func _init():
 ability_name = "Attack"
 ability_tag = &"ability.skill.attack"

 # Custo stamina para atacar
 cost_effect = preload("res://resources/player/effects/attack_stamina_cost.tres")

 # Cooldown para evitar spam
 cooldown_effect = preload("res://resources/player/effects/attack_cooldown.tres")

 # Efeito de dano para inimigos
 effect = preload("res://resources/player/effects/attack_damage.tres")

 # Tags para gerenciamento de estado
 activation_owned_tags = [&"state.attacking"]
 activation_blocked_tags = [&"state.stunned"]

 # Feedback visual/sonoro
 cues = [preload("res://resources/player/cues/attack_cue.tres")]
```

### Passo 4: Criar Cues de Animação

Crie `walk_cue.tres`:

```gdscript
extends AbilitySystemCueAnimation

func _init():
 cue_tag = &"cue.animation.walk"
 animation_name = "walk"
```

Crie `attack_cue.tres`:

```gdscript
extends AbilitySystemCueAnimation

func _init():
 cue_tag = &"cue.animation.attack"
 animation_name = "attack"
```

### Passo 5: Criar Efeitos

Crie `attack_stamina_cost.tres`:

```gdscript
extends AbilitySystemEffect

func _init():
 # Custo 10 stamina
 add_modifier(&"Stamina", MODIFIER_OP.ADD, -10.0)
```

Crie `attack_cooldown.tres`:

```gdscript
extends AbilitySystemEffect

func _init():
 # Cooldown de 0.5 segundos
 duration = 0.5
 add_granted_tag(&"cooldown.attack")
```

### Passo 6: Criar Container do Player

Crie `player_ability_container.tres`:

```gdscript
extends AbilitySystemAbilityContainer

func _init():
 # Define atributos
 attribute_set = preload("res://resources/player/player_attribute_set.tres")

 # Adiciona habilidades
 abilities = [
  preload("res://resources/player/abilities/walk.tres"),
  preload("res://resources/player/abilities/idle.tres"),
  preload("res://resources/player/abilities/attack.tres")
 ]
```

### Passo 7: Configurar Cena do Player

Crie script do player:

```gdscript
extends CharacterBody2D

const SPEED = 200.0

@onready var asc: AbilitySystemComponent = $AbilitySystemComponent
@onready var sprite: AnimatedSprite2D = $AnimatedSprite2D
@onready var attack_area: Area2D = $Area2D
@onready var audio_player: AudioStreamPlayer2D = $AudioStreamPlayer2D

var is_attacking = false
var is_dead = false

func _ready() -> void:
 # Configura feedback de áudio
 asc.set_audio_player(audio_player)

 # Aplica container do player (atributos + habilidades)
 var container = preload("res://resources/player/player_ability_container.tres")
 asc.apply_ability_container(container)

 # Conecta sinais
 asc.attribute_changed.connect(_on_attribute_changed)
 asc.ability_activated.connect(_on_ability_activated)

func _physics_process(delta: float) -> void:
 if is_dead: return

 # Lida com input de ataque
 if Input.is_action_just_pressed("attack") and not is_attacking:
  asc.try_activate_ability(&"ability.skill.attack")

 # Lida com movimento com sistema de habilidades
 var direction := Input.get_vector("move_left", "move_right", "move_up", "move_down")

 if direction:
  # Tenta ativar habilidade walk se não estiver ativa
  if not asc.has_tag(&"ability.skill.walk"):
   asc.try_activate_ability(&"ability.skill.walk")

  # Move apenas se habilidade walk estiver ativa e não estiver atacando
  if asc.has_tag(&"ability.skill.walk") and not is_attacking:
   velocity.x = direction.x * SPEED
   velocity.y = direction.y * SPEED
   sprite.flip_h = direction.x < 0
 else:
  # Tenta ativar idle quando não se move
  if not asc.has_tag(&"ability.skill.idle"):
   asc.try_activate_ability(&"ability.skill.idle")

  velocity.x = move_toward(velocity.x, 0, SPEED)
  velocity.y = move_toward(velocity.y, 0, SPEED)

 move_and_slide()

func _on_attribute_changed(attr: StringName, old_val: float, new_val: float) -> void:
 match attr:
  &"Health":
   if new_val <= 0:
    die()
   update_health_bar(new_val)
  &"Stamina":
   update_stamina_bar(new_val)

func _on_ability_activated(spec: AbilitySystemAbilitySpec) -> void:
 match spec.ability.ability_tag:
  &"ability.skill.attack":
   is_attacking = true
   # Lida com lógica de ataque (dano a inimigos, etc.)
   handle_attack()

func die():
 is_dead = true
 # Toca animação de morte, desabilita input, etc.

func handle_attack():
 # Verifica inimigos na área de ataque
 for body in attack_area.get_overlapping_bodies():
  if body.has_method("take_damage"):
   # Cria spec de efeito de dano
   var damage_effect = preload("res://resources/player/effects/attack_damage.tres")
   var damage_spec = asc.make_outgoing_spec(damage_effect, 1.0)

   # Aplica dano ao inimigo
   body.take_damage(damage_spec)
```

### Passo 8: Configurar Input Map

Em Configurações do Projeto > Input Map, adicione:

- `move_left`: Tecla A, Seta esquerda
- `move_right`: Tecla D, Seta direita
- `move_up`: Tecla W, Seta cima
- `move_down`: Tecla S, Seta baixo
- `attack`: Tecla Espaço

### Resultado

Agora você tem um player 2D completo com:

- **Sistema de health e stamina baseado em atributos**
- **Movimento direcionado por habilidades** (estados walk/idle)
- **Sistema de combate** com custos de stamina e cooldowns
- **Feedback de animação** via sistema Cue
- **Arquitetura extensível** para adicionar mais habilidades

Isso demonstra o poder central do Ability System: gameplay orientado a dados onde mecânicas são definidas como recursos em vez de lógica hardcodada.

---

_Este documento define a especificação técnica oficial do módulo Ability System._
