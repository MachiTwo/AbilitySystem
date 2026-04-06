import os
import re


def fix_godot_files(root_dir):
    # Tags to strictly normalize
    tag_prefixes = ["state.", "attack.", "effect.", "weapon.", "package."]
    attributes = [
        "health",
        "stamina",
        "acceleration",
        "friction",
        "air_resistance",
        "jump_force",
        "gravity_scale",
        "max_speed",
        "coyote_time",
        "jump_buffer_time",
        "stamina_regen",
        "dash_cost",
        "dash_attack",
    ]

    for root, dirs, files in os.walk(root_dir):
        if ".godot" in root or "godot-cpp" in root:
            continue

        for file in files:
            if file.endswith((".tres", ".tscn", ".godot", ".gd")):
                file_path = os.path.join(root, file)
                try:
                    with open(file_path, "r", encoding="utf-8") as f:
                        content = f.read()

                    original_content = content

                    # 1. Global tag fix (Ensure &"tag" or "tag" always has quotes)
                    for prefix in tag_prefixes:
                        # Case: &prefix.word -> &"prefix.word"
                        content = re.sub(
                            rf"&({re.escape(prefix)}[a-z_0-9]*)",
                            r'&"\1"',
                            content,
                            flags=re.IGNORECASE,
                        )
                        # Case: "prefix.word" -> "prefix.word" (ensure lowercase)
                        content = re.sub(
                            rf'"{re.escape(prefix)}[^"]*"',
                            lambda m: m.group(0).lower(),
                            content,
                            flags=re.IGNORECASE,
                        )
                        content = re.sub(
                            rf"'{re.escape(prefix)}[^']*'",
                            lambda m: m.group(0).lower(),
                            content,
                            flags=re.IGNORECASE,
                        )

                    # 2. Global attribute fix (Ensure attributes have quotes/type hints)
                    for attr in attributes:
                        # Fixes: = health -> = "health" OR = &health -> = &"health"
                        content = re.sub(
                            rf'=\s*(?!&")(?!"){attr}\b', f'= "{attr}"', content
                        )
                        content = re.sub(rf"=\s*&{attr}\b", f'= &"{attr}"', content)
                        # Fixes: : health -> : "health" (Inside dictionaries as values)
                        content = re.sub(
                            rf':\s*(?!&")(?!"){attr}\b', f': "{attr}"', content
                        )
                        content = re.sub(rf":\s*&{attr}\b", f': &"{attr}"', content)
                        # Fixes: &health: -> &"health": (Inside dictionaries/snapshots as KEYS)
                        content = re.sub(rf"&{attr}\b\s*:", f'&"{attr}":', content)

                    # 3. Standardize underscores (dashattack -> dash_attack)
                    content = content.replace("dashattack", "dash_attack")
                    content = content.replace("dash_attack", "dash_attack")
                    content = content.replace("dashcost", "dash_cost")

                    # 4. Remove UIDs from ext_resource to force path-based loading
                    content = re.sub(r' uid="uid://[^"]*"', "", content)

                    # 5. Fix common corrupted assignments from previous failed scripts
                    for attr in attributes:
                        content = re.sub(
                            rf'/\s*attribute\s*=\s*(?!&")(?!"){attr}\b',
                            f'/attribute = &"{attr}"',
                            content,
                        )

                    if content != original_content:
                        with open(file_path, "w", encoding="utf-8") as f:
                            f.write(content)
                        print(f"Fixed: {file_path}")

                except Exception as e:
                    print(f"Error processing {file_path}: {e}")


if __name__ == "__main__":
    fix_godot_files("demo")
