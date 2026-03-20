#!/usr/bin/env python
"""
download_integration.py

Downloads integration dependencies (e.g. LimboAI) into the ROOT addons/ folder.

Architecture:
   addons/
     ability_system/   <- compiled by SCons
     limboai/          <- downloaded by this script

   post_build.py then copies BOTH addons to all demo projects.

Usage:
    # Default: installs to <project_root>/addons/<plugin_name>/
    python utility/download_integration.py

    # Override destination (must be an existing project root):
    python utility/download_integration.py --target-addons path/to/custom/addons

Reads version metadata from deps.env at the project root.
"""

import os
import sys
import shutil
import zipfile
import argparse
import urllib.request
import urllib.error
import time

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def parse_env_file(path: str) -> dict:
    """Parse a simple KEY=VALUE .env file (ignores comments and blank lines)."""
    result = {}
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if "=" in line:
                key, _, value = line.partition("=")
                result[key.strip()] = value.strip()
    return result


def download_with_retry(url: str, dest: str, tries: int = 5, wait: int = 5) -> None:
    """Download a URL to dest, retrying on failure."""
    last_error = None
    for attempt in range(1, tries + 1):
        try:
            print(f"  [Attempt {attempt}/{tries}] Downloading {url} ...")
            urllib.request.urlretrieve(url, dest)
            print(f"  Downloaded → {dest}")
            return
        except (urllib.error.URLError, OSError) as e:
            last_error = e
            print(f"  [WARNING] Attempt {attempt} failed: {e}")
            if attempt < tries:
                print(f"  Waiting {wait}s before retry...")
                time.sleep(wait)

    raise RuntimeError(
        f"Failed to download {url} after {tries} attempts. Last error: {last_error}"
    )


def find_plugin_dir(
    unpack_root: str, plugin_name: str, max_depth: int = 3
) -> str | None:
    """Walk up to max_depth levels to find a directory named plugin_name."""
    for root, dirs, _ in os.walk(unpack_root):
        depth = root.replace(unpack_root, "").count(os.sep)
        if depth >= max_depth:
            dirs.clear()  # Prune walk — don't go deeper
            continue
        for d in dirs:
            if d == plugin_name:
                return os.path.join(root, d)
    return None


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Download and install integration dependencies (e.g. LimboAI) into root addons/."
    )
    parser.add_argument(
        "--target-addons",
        default=None,
        help=(
            "Override destination addons directory (absolute or relative to project root). "
            "Default: <project_root>/addons/"
        ),
    )
    parser.add_argument(
        "--deps-env",
        default=None,
        help="Path to deps.env file. Default: auto-detected from project root.",
    )
    parser.add_argument(
        "--tmp-dir",
        default="tmp-integration-dl",
        help="Temporary directory for downloads. Default: tmp-integration-dl",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Remove temporary download folder after installation.",
    )
    args = parser.parse_args()

    # Resolve project root (this script lives in utility/)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    # Load deps.env
    deps_env_path = args.deps_env or os.path.join(project_root, "deps.env")
    if not os.path.exists(deps_env_path):
        print(f"[ERROR] deps.env not found at: {deps_env_path}")
        return 1

    env = parse_env_file(deps_env_path)

    # Required keys
    required = [
        "PROJECT_INTEGRATION_NAME",
        "PROJECT_INTEGRATION_GITHUB",
        "PROJECT_INTEGRATION_VERSION",
        "PROJECT_INTEGRATION_PACKAGE_PLUGIN",
    ]
    missing = [k for k in required if k not in env]
    if missing:
        print(f"[ERROR] Missing keys in deps.env: {', '.join(missing)}")
        return 1

    plugin_name = env["PROJECT_INTEGRATION_NAME"]  # e.g. "limboai"
    github_repo = env["PROJECT_INTEGRATION_GITHUB"]  # e.g. "limbonaut/limboai"
    version = env["PROJECT_INTEGRATION_VERSION"]  # e.g. "1.7.0"
    package_name = env[
        "PROJECT_INTEGRATION_PACKAGE_PLUGIN"
    ]  # e.g. "limboai+v1.7.0.gdextension-4.6.zip"

    # Build URL
    url = (
        f"https://github.com/{github_repo}/releases/download/v{version}/{package_name}"
    )

    # Resolve paths
    # Default: <project_root>/addons/ — same level as ability_system/
    # post_build.py propagates the whole addons/ tree to all demo projects.
    if args.target_addons:
        target_addons = (
            args.target_addons
            if os.path.isabs(args.target_addons)
            else os.path.join(project_root, args.target_addons)
        )
    else:
        target_addons = os.path.join(project_root, "addons")

    target_plugin = os.path.join(target_addons, plugin_name)
    tmp_dir = os.path.join(project_root, args.tmp_dir)
    zip_path = os.path.join(tmp_dir, f"{plugin_name}.zip")
    unpack_dir = os.path.join(tmp_dir, "unpacked")

    print(f"[DOWNLOAD INTEGRATION] Destination    : {target_addons}/")
    print(f"[DOWNLOAD INTEGRATION] Plugin         : {plugin_name} v{version}")
    print(f"[DOWNLOAD INTEGRATION] URL            : {url}")
    print(
        "[DOWNLOAD INTEGRATION] NOTE: Run utility/post_build.py afterwards to sync to demo projects."
    )

    # Idempotency check — skip if already installed AND seems complete.
    # We check for the 'bin' directory. If it's missing or nearly empty, we assume it's an incomplete
    # install (possibly just the .gdextension file tracked by git).
    bin_dir = os.path.join(target_plugin, "bin")
    is_complete = (
        os.path.isdir(target_plugin)
        and os.path.isdir(bin_dir)
        and len(os.listdir(bin_dir)) > 1
    )

    if is_complete:
        print(
            f"[DOWNLOAD INTEGRATION] ✅ {plugin_name} already present and seems complete at {target_plugin}. Skipping."
        )
        return 0

    if os.path.isdir(target_plugin):
        print(
            f"[DOWNLOAD INTEGRATION] ⚠️ {plugin_name} found but seems incomplete. Re-downloading..."
        )

    # Create temp dir
    os.makedirs(tmp_dir, exist_ok=True)
    os.makedirs(unpack_dir, exist_ok=True)
    os.makedirs(target_addons, exist_ok=True)

    # Download
    download_with_retry(url, zip_path)

    # Unpack
    print(f"[DOWNLOAD INTEGRATION] Unpacking {zip_path} → {unpack_dir} ...")
    with zipfile.ZipFile(zip_path, "r") as zf:
        zf.extractall(unpack_dir)

    # Find plugin folder inside the zip
    found_dir = find_plugin_dir(unpack_dir, plugin_name, max_depth=3)
    if not found_dir:
        print(
            f"[ERROR] Could not find '{plugin_name}' directory inside the downloaded zip."
        )
        print(f"        Contents of {unpack_dir}:")
        for root, dirs, files in os.walk(unpack_dir):
            depth = root.replace(unpack_dir, "").count(os.sep)
            if depth > 3:
                continue
            indent = "  " * depth
            print(f"{indent}{os.path.basename(root)}/")
        return 1

    print(f"[DOWNLOAD INTEGRATION] Found plugin at: {found_dir}")

    # Copy to target addons
    if os.path.isdir(target_plugin):
        shutil.rmtree(target_plugin)

    shutil.copytree(found_dir, target_plugin)
    print(f"[DOWNLOAD INTEGRATION] ✅ Installed {plugin_name} → {target_plugin}")

    # Ensure .gdextension file is at the root of the plugin folder (Godot 4 convention)
    # Sometimes plugins put it inside bin/, which can make discovery harder in headless/CI.
    for root, _, files in os.walk(target_plugin):
        for file in files:
            if file.endswith(".gdextension"):
                source_path = os.path.join(root, file)
                dest_path = os.path.join(target_plugin, file)
                if source_path != dest_path:
                    print(
                        f"[DOWNLOAD INTEGRATION] Moving {file} to addon root: {dest_path}"
                    )
                    if os.path.exists(dest_path):
                        os.remove(dest_path)
                    shutil.move(source_path, dest_path)

    # Cleanup
    if args.clean:
        shutil.rmtree(tmp_dir, ignore_errors=True)
        print(f"[DOWNLOAD INTEGRATION] Cleaned up temp dir: {tmp_dir}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
