# 🚀 Release Notes - v0.1.0 (Lançamento Oficial)

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**English**](release_note.md) | [**Português**](release_note.pt.md)

Temos o orgulho de anunciar o lançamento do **Ability System Framework**, uma solução madura, robusta e orientada a dados para Godot. Disponível tanto como plugin **GDExtension** quanto como **Módulo Nativo** (C++), o sistema traz o rigor da engenharia de features complexas para a sua arquitetura de jogo.

---

## 1. Orquestração e Entidades Base

- **`ASComponent` (O Cérebro)**: Anexado ao seu `CharacterBody`, opera exclusivamente no `physics_process` (garantindo determinismo). Domina os atributos e gerencia a lista de Specs ativos.

- **`ASContainer` (Template de Classes)**: Elimina a configuração manual. Ele age como um arquétipo para inicializar o componente inteiramente a partir de um `.tres`, facilitando a vida para estruturar um "Mago" ou "Guerreiro" baseando-se em inventários pré-selecionados de habilidades e atributos nativos.

- **`ASDelivery` & `ASPackage`**: Injetores de física pura! Uma `Area2D` ou `Area3D` agora propaga um pacote (`ASPackage`) contendo dezenas de magias em um golpe ao colidir de forma natural com o alvo, separando a matemática de dano do rastreio do alvo.

## 2. Recursos Reutilizáveis (Blueprints Imutáveis)

- **`ASAbility`**: O núcleo de uma habilidade, validando requisitos de nível, tags e custos.

- **`ASEffect`**: Modificadores de estado isolados com suporte primitivo a duração, cooldowns individuais e políticas precisas de empilhamento (Stacking).

- **`ASCue`**: Camada pura de reatividade visual. Extensões (como `ASCueAudio`) operam de forma independente sem afetar dados do servidor de jogo.

- **`ASAttribute`**: Estatísticas fundamentadas que reagem dinamicamente via **Attribute Drivers**. Mudar a base de uma força altera reflexamente o total de dano calculado.

## 3. A Soberania dos "Specs" (Runtime)

Enquanto a `ASAbility` é estática na pasta do projeto, o **`ASAbilitySpec`**, **`ASEffectSpec`**, **`ASCueSpec`** e **`ASTagSpec`** são instâncias ativas baseadas em `RefCounted`. Isso resolve de vez o problema de sobreposição! Oponentes de níveis distintos podem usar a exata mesma habilidade base — os Specs retêm e calculam o estado vigente baseado nos status isolados do conjurador, limpando da memória ao fim da utilidade.

---

## 🏷️ O Motor de Tags Globais

Não gerenciamos booleanos espalhados pelo código; gerenciamos **Tags**.

- **`AbilitySystem` (Singleton)**: O servidor global que estende o _Project Settings_. É o responsável por registrar as Tags na engine. É a única entidade que carrega e salva a lista global no `project.godot`.

- **NameTag & ConditionalTag**: As tags identificam a origem (`NameTag` ex: `Ability.Skill.Fireball`) e a semântica de dano ou estado (`ConditionalTag` ex: `Damage.Element.Fire`). Facilita enormemente bônus de resistência global (fogo, impacto) em vez de precisar rastrear nomes curtos de instâncias.

- **Regras Lógicas**: Condições completas como `Required All`, `Required Any`, `Blocked Any` e `Blocked All` são avaliadas nativamente no core C++.

- **Automação via Ability Triggers**: Habilidades agora reagem autonomamente às Tags globais, sem _scripting_ extra. Pode-se acionar escudos reflexos ou counter-attacks puramente atrelando o `ON_TAG_ADDED` ou `ON_TAG_REMOVED` de um debuff ou dano recebido.

## Padrões de API e Acesso Seguros

A interface do Ability System encoraja buscar recursos de forma blindada e previsível, focando em evitar mutações furtivas:

- **Instanciação Dupla (`by_tag` vs `by_resource`)**: Todo método possui duas assinaturas. Use `by_tag` para invocar ações dinâmicas baseadas nativamente no motor de Tags, ou `by_resource` para referências duras (Path/UID).

- **Execução Segura (`try_activate`)**: A API não expõe um `activate()` direto para habilidades funcionais. O uso mandatório de `try_activate` garante que regras do Blueprint (custo, cooldown, requisitos lógicos) sejam irrevogavelmente respeitadas antes de qualquer execução.

- **Autorização (`can_`)**: Avalia se uma ação tem permissão teórica para ser executada, sem processá-la.

- **Consulta de Estado (`is_` / `has_`)**: Checagens limpas e seguras para validar a posse de Tags (`has_tag`) ou status operacionais (`is_active`), substituindo o uso de variáveis booleanas soltas.

- **Interrupção (`cancel_`)**: Encerra ativamente fluxos em andamento de habilidades ou efeitos em andamento.

- **Intenção em Rede (`request_`)**: Metodologia formal para solicitar execuções via RPC, delegando a autoridade de fato para o servidor.

- **Extração Segura (`get_`)**: Obtém valores, instâncias e dados calculados de forma limpa sem expor mutabilidade indevida.

- **Gestão de Inventário (`unlock_` / `lock_`)**: Define a disponibilidade e o catálogo ativo das habilidades equipáveis para o ator em runtime.

---

## 🌐 Engine Multiplayer

Para instâncias online e jogos competitivos, adicionamos superpoderes práticos:

- **Multiplayer e Predição (Rollback)**:

- **`ASStateCache`**: Estrutura C++ leve (inline struct) projetada para reter histórico recente de ticks na memória para predição local otimizada.

- **`ASStateSnapshot`**: Captura literal (foto) de todo o status fundamental. Permite salvar em disco com métodos base Godot para um fluxo real de **Save/Load** e atua como a fonte de autoridade central (True State) quando o Servidor sinaliza correções e força um Rollback do Frame.

---

## 🧬 Interações e a Inteligência (LimboAI Bridge)

A inteligência de jogo foi re-estruturada para operar em simbiose com o nosso framework:

- **Ponte Nativa com LimboAI**: Árvores de comportamento podem controlar formalmente agentes subordinados que portam um `ASComponent`, ativando comandos e consumindo nós de `BTAction` com latência física precisa e sincronizada nativamente pela Godot.
