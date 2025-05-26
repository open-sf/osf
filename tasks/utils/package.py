import os
import logging
import json
import shutil
import git
import hashlib
import getpass

from os import path
from datetime import datetime

# Create a logger
logger = logging.getLogger(__name__)


def generate_hash_file(pkg_name: str) -> str:
    """Create a SHA1 hash file for the package"""

    logger.info(f"Generating hash file for {pkg_name}")

    buf_size = 65536
    sha1 = hashlib.sha1()

    # Strip file extension
    pkg_name = os.path.splitext(pkg_name)[0]

    with open(pkg_name + ".zip", "rb") as f:
        while True:
            data = f.read(buf_size)
            if not data:
                break
            sha1.update(data)

    with open(pkg_name + ".sha1", "w") as f:
        f.write(format(sha1.hexdigest()))

    return pkg_name + ".sha1"


def generate_manifest(board: str, target: str) -> dict:
    """Generate manifest file from current build directory"""

    # Save build date/time
    now = datetime.now()
    build_date = now.strftime("%Y-%m-%dT%H:%M:%S")
    build_user = getpass.getuser()
    
    # Get project Git repo information
    prj_path = os.getcwd()
    git_repo = git.Repo(prj_path)
    git_describe = git_repo.git.describe("--tags", "--long", "--always", "--dirty=+")
    git_commit_sha = git_repo.head.object.hexsha

    try:
        git_branch = git_repo.active_branch.name
    except TypeError:
        # HEAD is datached (e.g. a git tag)
        git_branch = 'DETACHED_' + git_commit_sha

    manifest = {
        "image": {
            "board": board,
            "version": git_describe,
            "target": target
        },
        "build": {
            "user": build_user,
            "date": build_date,
        },
        "git": {
            "describe": git_describe,
            "commit": git_commit_sha,
            "branch": git_branch
        },
    }
    logger.debug(f"Manifest: {manifest}")
    return manifest


def _is_version_clean(version: str):
    """Determine if app version which is derived from 'git describe' is a clean (release candidate) repo or not"""

    if version[:-1] == "+" or "dirty" in version:
        # Dirty repo
        return False

    if "-" not in version:
        # Handle case where repo has not been tagged
        return False

    build = version.split("-")[1]
    if build != "0":
        # Number of commits ahead of tag is > 0
        return False
    return True


def _strip_version_metadata(version: str):
    """Take a 'git describe' output and return just the vMAJOR.MINOR.PATCH"""
    return version.split("-")[0]


def generate_package(board: str, target: str):
    """Generate factory firmware package from build directory"""

    manifest = generate_manifest(board, target)

    app_version = manifest['image']['version']
    if _is_version_clean(app_version):
        app_version = _strip_version_metadata(app_version)

    pkg_name = f"netloc_{manifest['image']['board']}_{app_version}"
    logger.info(f"Generating package: {pkg_name}.zip")

    build_path = path.join("examples", "osf-flock", "build", f"{target}", f"{board}")
    
    file_paths = []
    file_paths.append(path.join(build_path, f"node.elf"))
    file_paths.append(path.join(build_path, f"node.hex"))
    file_paths.append(path.join(build_path, f"node.bin"))
    file_paths.append(path.join(build_path, f"node.{target}"))

    manifest["image"]["hex_path"] = "node.hex"

    # Create folder for storing files to be zipped
    if os.path.exists(pkg_name):
        logger.warning(f"Folder '{pkg_name}' already exists, removing")
        shutil.rmtree(pkg_name)
    if os.path.exists(pkg_name + ".zip"):
        logger.warning(f"Package '{pkg_name}.zip' already exists, removing")
        os.remove(pkg_name + ".zip")
    os.makedirs(pkg_name, exist_ok=False)

    for file in file_paths:
        shutil.copy(file, pkg_name)

    with open(path.join(os.getcwd(), pkg_name, "manifest.json"), "w") as out:
        out.write(json.dumps(manifest, indent=4))

    shutil.make_archive(pkg_name, "zip", pkg_name)
    shutil.rmtree(pkg_name)
    return pkg_name + ".zip"