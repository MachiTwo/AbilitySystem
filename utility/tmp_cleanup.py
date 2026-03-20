import os
import sys

# Detect project root dynamically
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
SRC_DIR = os.path.join(PROJECT_ROOT, "src")
FOLDERS = ["core", "resources", "scene", "editor", "tests"]


def clean_file(filepath):
    if not os.path.exists(filepath):
        return False

    with open(filepath, "r", encoding="utf-8") as f:
        lines = f.readlines()

    new_lines = []
    i = 0
    modified = False

    while i < len(lines):
        line = lines[i]
        strip_l = line.strip()

        # Se encontrarmos o início de QUALQUER bloco do sistema, entramos em modo de extração
        if (
            strip_l == "#ifdef ABILITY_SYSTEM_MODULE"
            or strip_l == "#ifdef ABILITY_SYSTEM_GDEXTENSION"
        ):
            j = i + 1
            depth = 1
            elif_idx = -1
            while j < len(lines):
                sj = lines[j].strip()
                if sj.startswith("#if"):
                    depth += 1
                elif sj.startswith("#elif") or sj.startswith("#else"):
                    if depth == 1:
                        elif_idx = j
                elif sj.startswith("#endif"):
                    depth -= 1
                    if depth == 0:
                        break
                j += 1

            if j < len(lines):
                # Se houver uma parte "Alcohol" (elif/else), pegamos ela recursivamente
                if elif_idx != -1:
                    # Parte de baixo do bloco (GDEX)
                    content = lines[elif_idx + 1 : j]
                else:
                    # Parte de cima (Module apenas)
                    content = lines[i + 1 : j]

                # Salvamos o conteúdo (sem as bordas do ifdef) para processamento futuro
                new_lines.extend(content)
                i = j + 1
                modified = True
                continue

        new_lines.append(line)
        i += 1

    if modified:
        # Se modificamos, rodamos de novo para limpar aninhamentos
        content = "".join(new_lines)
        with open(filepath, "w", encoding="utf-8", newline="\n") as f:
            f.write(content)
        # Recursão para garantir que limpou TUDO (aninhamentos complexos)
        clean_file(filepath)
        return True
    return False


if __name__ == "__main__":
    count = 0
    files_to_check = sys.argv[1:]
    if files_to_check:
        for f in files_to_check:
            abs_path = os.path.abspath(f)
            if clean_file(abs_path):
                count += 1
    else:
        for folder in FOLDERS:
            dir_path = os.path.join(SRC_DIR, folder)
            if not os.path.exists(dir_path):
                continue
            for root, dirs, files in os.walk(dir_path):
                for file in files:
                    if file.endswith((".cpp", ".h")):
                        if clean_file(os.path.join(root, file)):
                            count += 1
    if count > 0:
        print(f"Recursively cleaned {count} files to raw state.")
