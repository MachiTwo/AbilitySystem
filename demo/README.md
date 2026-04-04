# Demo - Ability System v4.6

Cena de demonstração funcional do **Ability System** - um framework de gerenciamento de habilidades e atributos para Godot 4.6+.

## 🚀 Quick Start

Abra qualquer uma destas cenas para começar:

- **`demo.tscn`** - Cena principal da demo
- **`player/player.tscn`** - Cena isolada do player
- **`ui/hud.tscn`** - Interface de usuário

Pressione **Play (F5)** para executar.

### Controles

| Tecla | Ação |
|-------|------|
| **WASD** | Movimentação (Idle/Walk/Run) |
| **Space** | Pulo (Jump/Fall) |
| **Mouse Click** | Ataque/Combo |
| **Shift + Click** | Dash Attack |

## 📚 Documentação

Para entender como a demo funciona, consulte:

- **[ABILITY_SYSTEM_DEMO.md](ABILITY_SYSTEM_DEMO.md)** - Guia completo da arquitetura e implementação
- **[Framework Documentation](../src/doc_classes/)** - Documentação técnica da API
- **[Business Rules (Português)](../BUSINESS_RULES.pt.md)** - Regras de design do framework
- **[Implementation Plan](../IMPLEMENTATION_PLAN.md)** - Plano de implementação e roadmap

## 📂 Estrutura

```
demo/
├── demo.tscn                    ← Cena principal
├── player/                      ← Player logic & assets
│   ├── player.tscn
│   ├── player.gd
│   └── resources/
├── ui/                          ← Interface
│   ├── hud.tscn
│   └── hud.gd
├── resources/                   ← Ability System data
│   ├── abilities/
│   ├── attributes/
│   ├── effects/
│   ├── cues/
│   └── container/
├── autoload/                    ← Global managers
│   └── GameMachine.gd
└── scenes/                      ← Level scenes
    └── level.tscn
```

## 🎮 Gameplay

Esta demo demonstra:

- ✅ Movimentação dinâmica (Idle → Walk → Run → Jump → Fall)
- ✅ Sistema de ataque com combos (3x Light → Heavy)
- ✅ Recursos (Stamina) com regeneração
- ✅ Tags dinâmicas para rastreamento de estado
- ✅ UI reativa sincronizada com estado do player
- ✅ Animações sincronizadas com estado

## 🔧 Técnicas Utilizadas

- **ASComponent**: Hub central de lógica do actor
- **Abilities**: Habilidades ativadas por input/eventos
- **Attributes**: Sistema de valores (health, stamina, etc)
- **Tags**: Rastreamento de estado (NAME, CONDITIONAL, EVENT)
- **Signals**: Atualização reativa de UI via `tag_changed`
- **Signal Callbacks**: HUD conectado a sinais, sem polling

## 🐛 Debugging

Logs habilitados para verificação:

```gdscript
# Em demo/player/player.gd
[State] Activating motion.idle
[State] Activating motion.walk
[Tag Color] Player color changed to: blue

# Em demo/ui/hud.gd
[HUD] tag_changed: ability.combo1 added
[HUD] State updated: idle → walk
[HUD] Weapon updated: unarmed
```

Verifique a **Output Console** do Godot durante execução.

## ⚡ Performance

- **Zero Polling**: UI atualiza apenas quando estado realmente muda
- **Signal-Based**: Uso de Godot signals para reatividade
- **Efficient Queries**: Busca de atributos em O(1) com HashMap

## 📖 Próximas Etapas

Para expandir a demo:

1. **Adicionar Habilidades**: Crie `.tres` em `resources/abilities/`
2. **Adicionar Efeitos**: Crie `.tres` em `resources/effects/`
3. **Adicionar Inimigos**: Use ASComponent para IA
4. **Melhorar UI**: Expanda `hud.gd` com novos painéis
5. **Adicionar Persistência**: Use ASStateSnapshot para save/load

## 📞 Suporte

Para dúvidas sobre a implementação, consulte:

- [ABILITY_SYSTEM_DEMO.md](ABILITY_SYSTEM_DEMO.md) - Arquitetura completa
- [../IMPLEMENTATION_PLAN.md](../IMPLEMENTATION_PLAN.md) - Roadmap do projeto
- [../BUSINESS_RULES.pt.md](../BUSINESS_RULES.pt.md) - Regras de design

---

**Versão**: 4.6 | **Godot**: 4.2+ | **Language**: GDScript + C++
