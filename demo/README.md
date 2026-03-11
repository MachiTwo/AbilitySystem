# Ability System Plugin - Demo

> [!TIP]
> **Read this in other languages / Leia isto em outros idiomas:**
> [**Português**](README.pt.md) | [**English**](README.md)

Este é um projeto de demonstração minimalista para o **Ability System Plugin**, uma ferramenta poderosa e modular baseada em C++ (GDExtension) para o gerenciamento de habilidades, atributos e estados em jogos desenvolvidos na Godot Engine 4.

## 🚀 Visão Geral

O objetivo desta demo é ilustrar como integrar o componente central do sistema (`AbilitySystemComponent`) em atores (Player e Inimigos) em um ambiente 2D. O projeto serve como um ponto de partida para testar a lógica de combate, buffs, debuffs e interações entre entidades.

## 📁 Estrutura do Projeto

Abaixo está uma descrição breve da organização dos diretórios:

- **`actors/`**: Contém as cenas e scripts dos personagens principais.
  - `player/`: Implementação do jogador.
  - `enemys/`: Protótipos de inimigos (ex: Orc).
- **`addons/ability_system/`**: O core do plugin. Contém os binários do GDExtension e a documentação técnica do sistema.
- **`assets/`**: Recursos visuais, incluindo sprites de personagens da série "Soldier" e "Orc", além de tilsets para o ambiente.
- **`resources/`**: Recursos do sistema, incluindo habilidades, atributos e efeitos.
- **`levels/`**: Cenas de nível. A cena principal para testes é a `sala_teste.tscn`.
- **`ui/`**: Interface de usuário, incluindo o HUD de saúde.

## 🧠 Componentes Chave

Cada ator na cena (`player` e `orc`) possui um **`AbilitySystemComponent`** (ASC) anexado.

No ASC, é onde toda a "mágica" acontece:

- **Gerenciamento de Atributos**: HP, Stamina, Mana, etc.
- **Processamento de Habilidades**: Ativação via Tags, controle de custos e cooldowns.
- **Manipulação de Efeitos**: Aplicação de modificadores temporários ou permanentes.
- **Cues Visuais**: Gatilhos automáticos para animações e sons.

## 🛠️ Tecnologias Utilizadas

- **Godot Engine 4.6+** (Forward+ Rendering)
- **GDExtension**: Lógica do sistema em C++ de alta performance.
- **Jolt Physics**: Integrado nas configurações do projeto para simulação física otimizada.

## 🕹️ Como Executar

1. Certifique-se de que o plugin está ativado em `Projeto > Configurações do Projeto > Plugins`.
2. Abra a cena `res://levels/sala_teste.tscn`.
3. Execute o projeto (`F5`) para testar a movimentação e a presença dos componentes de habilidade nos atores.

---

*Desenvolvido como parte do ecossistema Zyris Engine.*
