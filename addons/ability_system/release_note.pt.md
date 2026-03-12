# Release Notes - v0.1.0 (Stable)

> [!TIP]
> **Leia isto em outros idiomas / Read this in other languages:**
> [**Português**](release_note.pt.md) | [**English**](release_note.md)

Esta versão marca o lançamento estável (v0.1.0) do Ability System, consolidando a transição da versão de desenvolvimento (v0.1.0.dev). O foco principal desta release é a simplificação da API, expansão de funcionalidades reativas e estabilidade garantida por testes.

## 🚀 Novidades e Mudanças (v0.1.0.dev -> v0.1.0)

- **Renomeação de Classes (Prefixo AS)**: Todas as classes do sistema agora utilizam o prefixo curto `AS` em vez do extenso `AbilitySystem` (ex: `ASAbility`, `ASComponent`, `ASEffect`).
  - _Nota: O Singleton principal permanece como `AbilitySystem` para clareza e acesso global._
- **ASDelivery**: Introdução de um sistema leve para entrega de efeitos, ideal para projéteis e áreas de impacto (AoE).
- **ASPackage**: Novo container de dados reutilizável que agrupa múltiplos efeitos e cues. Atua como o payload oficial para o `ASDelivery`, permitindo a transmissão de efeitos complexos em um único recurso.
- **Ability Triggers**: As habilidades agora podem ser disparadas automaticamente através de mudanças em Tags (ex: ativar "Hurt" ao receber a tag `state.hurt`).
- **Integração com LimboAI**: Nova demo integrada demonstrando o uso de Behavior Trees para controlar agentes que utilizam o Ability System.
- **Suite de Testes (100% Core)**: Cobertura completa da lógica core através de testes unitários (`doctest`), garantindo que refatorações não quebrem o sistema.

## 🛠️ Refatorações e Limpeza

- **Remoção do `target_node`**: Simplificação drástica das assinaturas de métodos de ativação. O sistema agora foca no fluxo reativo e em metadados, deixando a lógica de "alvo" para ser tratada pela implementação específica do jogo ou via `ASDelivery`.
- **Melhorias na UI**: O editor de Tags e Atributos foi otimizado para a nova nomenclatura.

## 📦 Instalação

Consulte o [README.pt.md](README.pt.md) para instruções de instalação.
