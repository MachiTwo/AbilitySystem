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

    # 1. Copy from root's gdextension folder to addons/ability_system/
    config_src_dir = os.path.join(project_root, "gdextension")
    for file in [
        "ability_system.gdextension",
        "plugin.cfg",
    ]:
        src_file = os.path.join(config_src_dir, file)
        if os.path.exists(src_file):
            shutil.copy2(src_file, addons_dir)
            print(f"[POST-BUILD] Synced {file} to addons/ability_system/")
        else:
            print(f"[POST-BUILD] Warning: Source {file} not found in {config_src_dir}")

    # 2. Copy docs and readmes to addons/ability_system/
    for file in ["README.md", "README.pt.md", "release_note.md", "release_note.pt.md"]:
        src_file = os.path.join(project_root, file)
        if os.path.exists(src_file):
            shutil.copy2(src_file, addons_dir)

    # 3. Copy icons from source to addons/ability_system/icons/
    # Collect from both Ability System and LimboAI
    as_icons_dir = os.path.join(project_root, "src", "icons")
    limbo_icons_dir = os.path.join(project_root, "src", "limboai", "icons")
    addons_icons_dir = os.path.join(addons_dir, "icons")
    os.makedirs(addons_icons_dir, exist_ok=True)

    # Sync AS icons
    if os.path.exists(as_icons_dir):
        for svg_file in glob.glob(os.path.join(as_icons_dir, "*.svg")):
            shutil.copy2(svg_file, addons_icons_dir)

    # Sync LimboAI icons
    if os.path.exists(limbo_icons_dir):
        for svg_file in glob.glob(os.path.join(limbo_icons_dir, "*.svg")):
            shutil.copy2(svg_file, addons_icons_dir)
        print("[POST-BUILD] Synced LimboAI icons to addons/ability_system/icons/")

    # 4. Copy entire addons folder into the main demo project if it exists
    proj_path = os.path.join(project_root, "demo")
    if os.path.exists(proj_path):
        dst = os.path.join(proj_path, "addons")
        os.makedirs(dst, exist_ok=True)
        addons_src = os.path.join(project_root, "addons")
        if os.path.exists(addons_src):
            for item in os.listdir(addons_src):
                s = os.path.join(addons_src, item)
                d = os.path.join(dst, item)
                if os.path.isdir(s):
                    shutil.copytree(s, d, dirs_exist_ok=True)
                else:
                    shutil.copy2(s, d)


if __name__ == "__main__":
    sync_addons()
