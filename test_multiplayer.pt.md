# Framework de Testes Multiplayer

Este documento descreve a infraestrutura híbrida C++/Python usada para validar a sincronização multiplayer e a lógica de autoridade do Ability System.

## 🏗️ Arquitetura

O framework foi projetado para simular um ambiente multiplayer real sem a necessidade de uma árvore de cenas completa do Godot para cada teste.

- **Orquestrador (`utility/multiplayer/runner.py`)**: O ponto de entrada. Ele levanta o servidor mock e múltiplas instâncias de clientes.
- **State Mock Server (`mock_server.py`)**: Um servidor autoritativo leve que rastreia o estado global (Sets de Atributos, Tags) e valida as tentativas dos clientes de modificá-los.
- **Instâncias de Clientes (Godot Headless)**: Processos paralelos executando testes unitários `doctest`. Cada instância simula um arquétipo de entidade específico (Guerreiro, Mago, Boss, etc.).
- **Net Simulator (`net_simulator.py`)**: Injeta latência e perda de pacotes na comunicação local para testar a robustez do sistema.

## 🚀 Fluxo de Execução

1. **Setup**: O orquestrador limpa processos zumbis anteriores.
2. **Lançamento do Servidor**: O servidor mock Python inicia e se vincula à porta configurada.
3. **Spawning de Clientes**: Instâncias paralelas do Godot são lançadas com filtros de teste específicos (ex: `--test-case=*Player_1*`).
4. **Execução**: Os clientes realizam ações sincronizadas (ex: Aplicar um debuff no Jogador 2, verificar se o servidor replicou a tag).
5. **Coleta**: Os logs são salvos em `doctest_mp_player_N.log`.
6. **Análise**: O `log_analyzer.py` verifica se todos os 10 jogadores completaram seus cenários de cobertura de 300%.

## 🧪 Cenários Incluídos

- **Rejeição de Autoridade**: Cliente tentando mudar a vida sem permissão do servidor.
- **Propagação de Tags**: Tag de "Queima" aplicada em um alvo deve ser replicada para todos os observadores.
- **Drivers de Atributos em Sincronia**: Buff de Força no Servidor deve atualizar corretamente o Ataque nos Clientes.
- **Lógica de Late Joiner**: Um novo cliente entrando no meio do combate deve receber o estado atual de todos os `ASComponents`.
