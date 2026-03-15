# Release Notes - v0.1.0 (Estável)

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](release_note.md) | [**Português**](release_note.pt.md)

Esta versão marca o lançamento estável (v0.1.0) do Ability System, consolidando a transição da versão de desenvolvimento (v0.1.0.dev). O foco principal desta release é a simplificação da API, expansão de funcionalidades reativas e estabilidade absoluta garantida por testes e infraestrutura de build.

## 🚀 Novidades e Mudanças (v0.1.0.dev -> v0.1.0)

- **Renomeação de Classes (Prefixo AS)**: Todas as classes do sistema agora utilizam o prefixo curto `AS` em vez do extenso `AbilitySystem` (ex: `ASAbility`, `ASComponent`, `ASEffect`).
  - _Nota: O Singleton principal permanece como `AbilitySystem` para clareza e acesso global._
- **ASDelivery**: Introdução de um sistema leve para entrega de efeitos, ideal para projéteis e áreas de impacto (AoE).
- **ASPackage**: Novo container de dados reutilizável que agrupa múltiplos efeitos e cues. Atua como o payload oficial para o `ASDelivery`, permitindo a transmissão de efeitos complexos em um único recurso.
- **Ability Triggers**: As habilidades agora podem ser disparadas automaticamente através de mudanças em Tags (ex: ativar "Hurt" ao receber a tag `state.hurt`).
- **Integração com LimboAI**: Compatibilidade nativa com Behavior Trees para controlar agentes que utilizam o Ability System.
- **Suite de Testes (100% Core)**: Cobertura completa da lógica core através de testes unitários (`doctest`), garantindo segurança em refatorações.

## 🛠️ Infraestrutura e Polimento

- **Otimização de Build SCons**: Transição para o decider de cache baseado em **MD5**, garantindo builds incrementais confiáveis e consistência em CI/CD ao ignorar timestamps irrelevantes.
- **Revisão da Documentação**: `API.md` e `Tests.md` completamente reescritos para fornecer guias abrangentes e orientados à ação para desenvolvedores.
- **Remoção do `target_node`**: Simplificação das assinaturas de métodos de ativação. O sistema agora foca no fluxo reativo, delegando a lógica espacial de "alvo" para o `ASDelivery`.
- **Correção de Bug**: Resolvido um problema raro onde os `Ability Triggers` poderiam falhar ao utilizar containers duplicados dentro do `ASComponent`.

## 📦 Instalação

Consulte o [README.pt.md](README.pt.md) para instruções de instalação.
