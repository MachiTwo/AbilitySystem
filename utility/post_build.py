#!/usr/bin/env python
import os
import shutil
import glob


def sync_addons():
    src_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(src_dir)

    addons_dir = os.path.join(project_root, "addons", "ability_system")
    demo_addons_dir = os.path.join(project_root, "demo", "addons", "ability_system")

    os.makedirs(addons_dir, exist_ok=True)
    os.makedirs(demo_addons_dir, exist_ok=True)

    # 1. Copy from gdextension to addons/ability_system/
    for file in [
        "ability_system.gdextension",
        "plugin.cfg",
        "as_plugin.gd",
    ]:
        src_file = os.path.join(src_dir, file)
        if os.path.exists(src_file):
            shutil.copy2(src_file, addons_dir)

    # 2. Copy docs and readmes to addons/ability_system/
    for file in ["README.md", "README.pt.md", "release_note.md", "release_note.pt.md"]:
        src_file = os.path.join(project_root, file)
        if os.path.exists(src_file):
            shutil.copy2(src_file, addons_dir)

    # 3. Copy icons from source to addons/ability_system/icons/
    icons_dir = os.path.join(project_root, "src", "icons")
    addons_icons_dir = os.path.join(addons_dir, "icons")
    os.makedirs(addons_icons_dir, exist_ok=True)
    if os.path.exists(icons_dir):
        for svg_file in glob.glob(os.path.join(icons_dir, "*.svg")):
            shutil.copy2(svg_file, addons_icons_dir)

    # 4. Copy entire addons folder into demo projects
    # List of demo projects to sync
    demo_projects = ["demo", "demo-integration"]

    for proj in demo_projects:
        dst = os.path.join(project_root, proj, "addons")
        # Ensure destination exists
        os.makedirs(dst, exist_ok=True)

        addons_src = os.path.join(project_root, "addons")
        if os.path.exists(addons_src):
            # Copy all addons from root to demo projects, merging with existing ones
            for item in os.listdir(addons_src):
                s = os.path.join(addons_src, item)
                d = os.path.join(dst, item)
                if os.path.isdir(s):
                    # Use dirs_exist_ok=True (Python 3.8+) to merge folders instead of rmtree+copytree
                    try:
                        shutil.copytree(s, d, dirs_exist_ok=True)
                    except Exception as e:
                        print(f"Warning: Could not sync folder {item} to {proj}: {e}")
                else:
                    try:
                        shutil.copy2(s, d)
                    except Exception as e:
                        print(f"Warning: Could not sync file {item} to {proj}: {e}")


if __name__ == "__main__":
    sync_addons()
