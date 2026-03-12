# 🎮 Demo Integration: AbilitySystem + LimboAI

Esta demo é uma vitrine tecnológica que demonstra a integração profunda entre o **Ability System** e o **LimboAI Behavior Trees**. Ela não é apenas uma demonstração visual, mas uma implementação rigorosa de arquitetura de combate orientada a dados.

## 🏗️ Estrutura da Demo

A demo utiliza o `ASComponent` (ASC) como o motor de regras e estados dos agentes, enquanto o LimboAI atua como o motor de decisão estratégica.

### 1. Sistema de Tags (Espinha Dorsal)

- **Tags de Nome (`name_tags`)**: Definem habilidades (`ability.*`), efeitos (`effect.*`) e estados ativos (`state.attacking`, `state.burning`).
- **Tags Condicionais (`conditional_tags`)**: Lógica dinâmica calculada em tempo real.
  - `state.low_health`: < 40% HP.
  - `state.critical_health`: < 15% HP.
  - `state.immune`: Ativa se o atributo `Invulnerability` > 0.
  - `state.stunned`: Ativa se o atributo `StunTime` > 0.

### 2. Atributos e Efeitos

- **Atributos Core**: Health, MoveSpeed, AttackPower.
- **Buffs/Debuffs**:
  - `Burn`: DoT (Damage over Time) usando o sistema de `Period`.
  - `Haste`: Multiplicador de 1.5x em `MoveSpeed`.
  - `Attack Buff`: Multiplicador de 1.3x em `AttackPower`.

### 3. Orquestração de Cues

O feedback visual e sonoro é 100% gerenciado via Cues, desacoplado da lógica de negócio:

- **Animations**: `cue_idle`, `cue_walk`, `cue_attack`, `cue_hurt`, `cue_death`.
- **Audio (SFX)**: `cue_snd_attack`, `cue_snd_heal`, `cue_snd_hurt`, `cue_snd_fireball`, `cue_snd_walk`.

---

## 🧪 Protocolo TDD (Test-Driven Development)

O desenvolvimento deste plugin segue um ciclo rigoroso de **Red-Green-Refactor**:

1.  **RED (Falha)**: Escrevemos o teste em `src/tests` (ex: `test_as_mega_integration.h`) definindo o comportamento esperado (ex: uma tag condicional deve ser calculada corretamente).
2.  **GREEN (Sucesso)**: Implementamos o código mínimo necessário em C++ para o teste passar.
3.  **REFACTOR (Otimização)**: Refatoramos a implementação mantendo a integridade garantida pelos testes.

### Mega Integração

O teste `test_as_mega_integration.h` valida o "Caminho Feliz" completo:
`Ativação -> Consumo de Custo -> Execução de Task -> Disparo de Cue -> Aplicação de Efeito`.

---

## 🎵 Lista de Áudio (Requisitos)

Para o funcionamento pleno do feedback sonoro, a demo espera os seguintes arquivos em `res://demo-integration/assets/audio/`:

| Recurso Cue        | Arquivo Esperado | Evento                      |
| :----------------- | :--------------- | :-------------------------- |
| `cue_snd_attack`   | `attack.wav`     | OnExecute (Habilidade)      |
| `cue_snd_heal`     | `heal.wav`       | OnExecute (Efeito)          |
| `cue_snd_hurt`     | `hurt.wav`       | OnExecute (Atributo mudado) |
| `cue_snd_fireball` | `fireball.wav`   | OnExecute (Habilidade)      |
| `cue_snd_walk`     | `walk.wav`       | OnActive (Estado ativo)     |

---

## 🤖 Playtest Automatizado

Para rodar o teste de integração automática e gerar o log de resultados:

1. Inicie a cena `game.tscn`.
2. O `Game.gd` monitorará a vitória/derrota e salvará em `playtest_integration.log`.
3. O jogo fechará automaticamente ao fim da simulação.
