import os
import glob

doc_dir = "src/doc_classes"
xml_files = glob.glob(os.path.join(doc_dir, "*.xml"))

for xml_path in xml_files:
    with open(xml_path, "r", encoding="utf-8") as f:
        content = f.read()

    # 1. Fix ASEventTagTag typo
    content = content.replace("ASEventTagTag", "ASEventTag")

    # 2. Fix invalid inheritance "struct" and "class"
    content = content.replace('inherits="struct"', 'inherits="RefCounted"')
    content = content.replace('inherits="class"', 'inherits="RefCounted"')

    # 3. Fix unresolved type HashMap to Dictionary
    content = content.replace('type="HashMap"', 'type="Dictionary"')

    # 4. Handle specific issues in ASBridgeState.xml related to method links
    if "ASBridgeState.xml" in xml_path:
        content = content.replace("[method _enter]", "[code]_enter[/code]")
        content = content.replace("[method _exit]", "[code]_exit[/code]")
        content = content.replace("[method _update]", "[code]_update[/code]")
        # event_finished might be a method, but if it's missing, use code
        content = content.replace(
            "[method event_finished]", "[code]event_finished[/code]"
        )

    with open(xml_path, "w", encoding="utf-8") as f:
        f.write(content)

print(f"Processed {len(xml_files)} xml files.")
