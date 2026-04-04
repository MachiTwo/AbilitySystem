# Plano de Implementação - Infraestrutura de Testes Multiplayer

**Data**: 2026-04-04 | **Status**: Pronto para Implementação | **Prioridade**: Alta

---

## 📋 Visão Geral

Este plano aborda dois objetivos principais:

1. **Estabilização do Build**: Resolver erros persistentes de compilação em `test_as_component_rollback.h` consolidando arquivos duplicados e aplicando API pública
2. **Infraestrutura Multiplayer**: Estabelecer a infraestrutura de testes multiplayer no CI/CD, corrigindo erros de "arquivo não encontrado" para `runner.py` e `log_analyzer.py`

---

## ⚠️ Considerações Críticas

> **Assumções do Plano**:
>
> - O Ability System GDExtension implementa corretamente `AbilitySystem.run_tests()`
> - Pode ser invocado via Godot headless
> - Testes multiplayer rodarão 10 clientes simultâneos (matrix) coordenados por variáveis de ambiente

---

## 🔧 Fase 1: Estabilização do Build (Testes de Rollback)

### 1.1 Consolidação de Arquivos Duplicados

**Problema**:

- `test_as_component_rollback.h` existe em dois lugares
- Module build espera em `src/tests/`
- GDExtension pode estar buscando em `src/tests/unit/`

**Solução**:

#### Passo 1: Restaurar teste consolidado

Arquivo: `src/tests/test_as_component_rollback.h`

```cpp
// Versão única, corrigida, que funciona tanto em Module quanto GDExtension
#ifndef TEST_AS_COMPONENT_ROLLBACK_H
#define TEST_AS_COMPONENT_ROLLBACK_H

#include "test_tools.h"
#include "src/scene/as_component.h"

TEST_SUITE("ASComponent - Rollback") {
    TEST_CASE("Snapshot capture and restore") {
        // Implementação consolidada aqui
    }
}

#endif // TEST_AS_COMPONENT_ROLLBACK_H
```

#### Passo 2: Remover duplicata

Deletar: `src/tests/unit/test_as_component_rollback.h`

#### Passo 3: Atualizar referência em test_main.cpp

```cpp
#include "src/tests/test_as_component_rollback.h"  // ← Caminho consolidado
```

---

## 🌐 Fase 2: Infraestrutura Multiplayer (GitHub Actions)

### 2.1 Script Runner Python

**Arquivo**: `utility/multiplayer/runner.py`

Responsabilidades:

- Envolver execução do Godot com flags `--headless`
- Injetar `MP_GHA_PLAYER_ID` nas variáveis de ambiente
- Capturar saída para logs individuais
- Tratar sinais de forma limpa

```python
#!/usr/bin/env python3

import subprocess
import sys
import os
from pathlib import Path

def run_multiplayer_test(player_id: int, godot_bin: str, project_path: str = ".") -> int:
    """
    Executa um cliente multiplayer headless

    Args:
        player_id: ID único do jogador (0-9 para 10 clientes)
        godot_bin: Caminho do binário Godot
        project_path: Raiz do projeto Godot

    Returns:
        Código de saída do Godot
    """

    # Preparar logs
    log_dir = Path("utility/multiplayer/logs")
    log_dir.mkdir(parents=True, exist_ok=True)
    log_file = log_dir / f"doctest_mp_player_{player_id}.log"

    # Variáveis de ambiente
    env = os.environ.copy()
    env["MP_GHA_PLAYER_ID"] = str(player_id)
    env["MP_SERVER_PORT"] = "7777"
    env["MP_SERVER_HOST"] = "127.0.0.1"

    # Comando
    cmd = [
        godot_bin,
        "--path", project_path,
        "--headless",
        "-s", "mp_test_runner.gd"
    ]

    # Executar e capturar saída
    with open(log_file, "w") as f:
        try:
            result = subprocess.run(
                cmd,
                env=env,
                stdout=f,
                stderr=subprocess.STDOUT,
                timeout=300  # 5 minutos de timeout
            )
            return result.returncode
        except subprocess.TimeoutExpired:
            f.write("\n[TIMEOUT] Teste excedeu 5 minutos\n")
            return 1
        except Exception as e:
            f.write(f"\n[ERROR] {str(e)}\n")
            return 1

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--player-id", type=int, required=True)
    parser.add_argument("--godot-bin", default="Godot_v4.2.2-stable_linux.x86_64")
    parser.add_argument("--project-path", default=".")

    args = parser.parse_args()

    exit_code = run_multiplayer_test(
        args.player_id,
        args.godot_bin,
        args.project_path
    )

    sys.exit(exit_code)
```

### 2.2 Analisador de Logs Python

**Arquivo**: `utility/multiplayer/log_analyzer.py`

Responsabilidades:

- Escanear diretório de logs (`all-logs/`)
- Extrair contagens de sucesso/falha
- Gerar diagrama Mermaid se houver erros de sync
- Produzir relatório em markdown

````python
#!/usr/bin/env python3

import re
from pathlib import Path
from typing import Dict, List, Tuple

def analyze_logs(log_dir: str) -> Dict:
    """
    Analisa todos os logs de testes multiplayer

    Args:
        log_dir: Diretório contendo os arquivos .log

    Returns:
        Dicionário com estatísticas e relatórios
    """

    log_path = Path(log_dir)
    results = {
        "total_clients": 0,
        "passed": 0,
        "failed": 0,
        "errors": [],
        "sync_issues": []
    }

    for log_file in sorted(log_path.glob("doctest_mp_player_*.log")):
        results["total_clients"] += 1
        content = log_file.read_text()

        # Extrair resultados
        if "Test passed" in content or "SUCCESS" in content:
            results["passed"] += 1
        else:
            results["failed"] += 1
            results["errors"].append(str(log_file.name))

        # Detectar problemas de sync
        sync_errors = re.findall(r"\[SYNC_ERROR\].*", content)
        if sync_errors:
            results["sync_issues"].append({
                "client": log_file.name,
                "errors": sync_errors
            })

    return results

def generate_report(analysis: Dict) -> str:
    """Gera relatório em markdown"""

    report = f"""# Relatório de Testes Multiplayer

## Resumo
- **Total de Clientes**: {analysis['total_clients']}
- **Passou**: {analysis['passed']}
- **Falhou**: {analysis['failed']}
- **Taxa de Sucesso**: {analysis['passed'] / max(analysis['total_clients'], 1) * 100:.1f}%

"""

    if analysis["sync_issues"]:
        report += """## ⚠️ Problemas de Sincronização Detectados

"""
        for issue in analysis["sync_issues"]:
            report += f"- **{issue['client']}**: {len(issue['errors'])} erros\n"

        # Gerar diagrama Mermaid
        report += """
## Diagrama de Sincronização

```mermaid
sequenceDiagram
    participant Server
    participant Client1
    participant Client2
    Server->>Client1: sync_state
    Server->>Client2: sync_state
    Note over Server,Client2: Detectar divergências
````

"""

    return report

if **name** == "**main**":
import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--log-dir", required=True)
    parser.add_argument("--generate-mermaid", action="store_true")
    parser.add_argument("--gha-summary", action="store_true")

    args = parser.parse_args()

    analysis = analyze_logs(args.log_dir)
    report = generate_report(analysis)

    # Salvar relatório
    report_path = Path("utility/multiplayer/mp_coverage_report.md")
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(report)

    print(report)

    if args.gha_summary:
        # Adicionar ao GitHub Actions Step Summary
        summary_file = Path(os.environ.get("GITHUB_STEP_SUMMARY", "/tmp/summary.md"))
        with open(summary_file, "a") as f:
            f.write(report)

````

---

## 🎮 Fase 3: Integração com Godot

### 3.1 Script Runner de Testes Multiplayer

**Arquivo**: `demo/mp_test_runner.gd`

```gdscript
extends SceneTree

func _init():
    var player_id = _get_player_id()
    var is_multiplayer = player_id >= 0

    print("[MP Runner] Player ID: %d" % player_id)
    print("[MP Runner] Mode: %s" % ["Multiplayer" if is_multiplayer else "Unit"])

    if is_multiplayer:
        _run_multiplayer_test(player_id)
    else:
        _run_unit_test()

func _get_player_id() -> int:
    var env_id = OS.get_environment("MP_GHA_PLAYER_ID")
    return int(env_id) if env_id else -1

func _run_unit_test() -> void:
    print("[MP Runner] Executando testes unitários puros...")
    var result = AbilitySystem.run_tests()
    quit(result)

func _run_multiplayer_test(player_id: int) -> void:
    print("[MP Runner] Executando teste multiplayer - Player %d" % player_id)

    # Simular ambiente multiplayer
    var server_port = int(OS.get_environment("MP_SERVER_PORT", "7777"))
    var server_host = OS.get_environment("MP_SERVER_HOST", "127.0.0.1")

    # Client-side prediction test
    var peer = ENetMultiplayerPeer.new()
    var error = peer.create_client(server_host, server_port)

    if error != OK:
        print("[ERROR] Falha ao conectar ao servidor: %d" % error)
        quit(1)

    multiplayer.multiplayer_peer = peer

    # Executar testes integrados
    var result = AbilitySystem.run_tests()

    # Aguardar sincronização
    await get_tree().process_frame

    quit(result)
````

---

## 📝 Atualizações do GitHub Actions

### 3.1 Workflow test_multiplayer.yml

```yaml
name: 🌐 Multiplayer Tests

on:
  workflow_call:
    inputs:
      godot-bin-name:
        required: true
        type: string

jobs:
  run-multiplayer:
    name: 🌐 Executar Testes (Player ${{ matrix.player_id }})
    runs-on: ubuntu-latest
    strategy:
      fail-fast: false
      matrix:
        player_id: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

    steps:
      - name: Checkout
        uses: actions/checkout@v4.2.2

      - name: Setup Python
        uses: actions/setup-python@v5
        with:
          python-version: "3.11"

      - name: Restaurar Binários
        uses: actions/cache/restore@v4
        with:
          path: dist/
          key: ${{ github.sha }}-test-binaries
          fail-on-cache-miss: true

      - name: Deploy Binários
        run: |
          cp -r dist/standard/* .
          cp -r dist/godot/* .

      - name: Setup Godot
        run: |
          BIN_NAME="${{ inputs.godot-bin-name }}"
          chmod +x "$BIN_NAME"
          echo "GODOT_BIN=$(pwd)/$BIN_NAME" >> $GITHUB_ENV

      - name: Executar Teste Multiplayer
        env:
          MP_GHA_PLAYER_ID: ${{ matrix.player_id }}
          MP_SERVER_PORT: 7777
        run: |
          python utility/multiplayer/runner.py \
            --player-id ${{ matrix.player_id }} \
            --godot-bin "$GODOT_BIN"

      - name: Upload Log
        if: always()
        uses: actions/upload-artifact@v4
        with:
          name: mp-log-player-${{ matrix.player_id }}
          path: utility/multiplayer/logs/doctest_mp_player_${{ matrix.player_id }}.log

  analyze-multiplayer:
    name: 📊 Análise & Relatório
    needs: [run-multiplayer]
    runs-on: ubuntu-latest
    if: always() && needs.run-multiplayer.result != 'skipped'

    steps:
      - name: Checkout
        uses: actions/checkout@v4.2.2

      - name: Download Logs
        uses: actions/download-artifact@v4
        with:
          pattern: mp-log-player-*
          path: all-logs
          merge-multiple: false

      - name: Flatten Logs
        run: |
          find all-logs -name "*.log" -exec mv {} all-logs/ \; 2>/dev/null || true

      - name: Setup Python
        uses: actions/setup-python@v5
        with:
          python-version: "3.11"

      - name: Analisar Logs
        run: |
          python utility/multiplayer/log_analyzer.py \
            --log-dir all-logs \
            --generate-mermaid \
            --gha-summary

      - name: Publicar Resumo
        if: always()
        run: |
          if [ -f utility/multiplayer/mp_coverage_report.md ]; then
            cat utility/multiplayer/mp_coverage_report.md >> $GITHUB_STEP_SUMMARY
          fi
```

---

## 🤔 Questões Abertas & Soluções

### Problema: Coordenação de Rede

> Como os clientes encontram o servidor?

**Opção A**: IP local hardcoded

- ✅ Simples para CI/CD (containers isolados)
- ❌ Não testa rede real

**Opção B**: Servidor e clientes em threads separadas

- ✅ Testa sincronização real
- ❌ Mais complexo

**Solução Proposta** (Recomendado):

- Usar `127.0.0.1:7777` para CI/CD
- Server thread em `mp_test_runner.gd` se `MP_GHA_PLAYER_ID == 0`
- Outros clientes conectam ao server local

---

## ✅ Checklist de Implementação

### Fase 1: Build Stabilization

- [ ] Consolidar `test_as_component_rollback.h` em `src/tests/`
- [ ] Deletar duplicata em `src/tests/unit/`
- [ ] Atualizar referência em `test_main.cpp`
- [ ] Compilar e validar

### Fase 2: Scripts Python

- [ ] Criar `utility/multiplayer/runner.py`
- [ ] Criar `utility/multiplayer/log_analyzer.py`
- [ ] Testar localmente com mock logs

### Fase 3: Integração Godot

- [ ] Criar `demo/mp_test_runner.gd`
- [ ] Verificar projeto.godot tem extensão registrada
- [ ] Testar execução headless

### Fase 4: CI/CD

- [ ] Atualizar `test_multiplayer.yml`
- [ ] Atualizar `all_tests.yml` com nova dependência
- [ ] Testar workflow completo

### Validação Final

- [ ] GHA build passa (module + gdextension)
- [ ] 10 clientes multiplayer executam
- [ ] Logs são coletados e analisados
- [ ] Relatório Mermaid gerado se houver erro

---

## 📊 Estrutura de Arquivos Resultante

```
utility/multiplayer/
├── runner.py                          ← Novo: Executor de testes
├── log_analyzer.py                    ← Novo: Analisador de logs
├── mp_test_runner.gd                  ← Novo: Script Godot
├── logs/
│   ├── doctest_mp_player_1.log
│   ├── doctest_mp_player_2.log
│   └── ...
└── mp_coverage_report.md              ← Novo: Relatório gerado

.github/workflows/
├── test_multiplayer.yml               ← Atualizado
├── all_tests.yml                      ← Atualizado
└── test_infrastructure.yml            ← Sem mudanças
```

---

## 🚀 Próximos Passos

1. **Hoje**: Revisar este plano, coletar feedback
2. **Amanhã**: Implementar Fase 1 (Consolidação de Build)
3. **Dia 3**: Implementar Fases 2-3 (Scripts + Integração)
4. **Dia 4**: Testar e validar Fase 4 (CI/CD)
5. **Dia 5**: Deploy e monitoramento inicial

---

**Status**: Pronto para Execução | **Estimativa**: 4-5 dias | **Risco**: Baixo
