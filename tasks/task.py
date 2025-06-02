import os
import logging
import sys
import shutil

import inspect
if not hasattr(inspect, "getargspec"):
    inspect.getargspec = inspect.getfullargspec

from invoke import task
from rich.logging import RichHandler

cwd = os.getcwd()
sys.path.insert(0, os.path.join(cwd, "tasks"))
import utils.package
import utils.build

# Create a logger
logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

# Create a console handler
ch = RichHandler()
ch.setLevel(logging.DEBUG)

# Add handlers to the logger
logger.addHandler(ch)

# Add utils logging
utils_logger = logging.getLogger("utils")
utils_logger.setLevel(logging.DEBUG)
utils_logger.addHandler(ch)

DEFAULT_BOARDS = [
    # (BOARD, TARGET)
    # ("dwm1001", "nrf52832"),
    # ("dwm3001cdk", "nrf52833"),
    # ("bluebite", "nrf52840"),
    # ("uwpcie", "nrf52840"),
    # ("dc10", "nrf52840"),
    ("dk", "nrf52840")
]

def _board_to_target(board: str) -> str:
    """Convert board name to target board name used in compiling"""
    for b in DEFAULT_BOARDS:
        if b[0] == board:
            return b[1]
    raise ValueError(f"Did not find {board} in {DEFAULT_BOARDS}")

@task(
    iterable=["boards"],
    help={
        "boards": "List of boards",
        "output": "Output directory",
    },
)
def package(ctx, boards: str = None, output: str = None):
    """Generate firmware package"""

    if boards == []:
        # User did not supply a list of boards, build for all default boards
        boards = DEFAULT_BOARDS

    # Build package for each board
    assets = []
    for b in boards:

        if type(b) == tuple:
            board = b[0] 
            target = b[1]
        else:
            # User provided list of board names
            board = b
            target = _board_to_target(b)

        utils.build.build_project(board, target)
        pkg = utils.package.generate_package(board, target)
        sha = utils.package.generate_hash_file(pkg)
        assets.append(pkg)
        assets.append(sha)

    if output != None:
        if os.path.exists(output):
            logger.warning(f"{output} exists, removing first")

        shutil.rmtree(output, ignore_errors=True)
        os.mkdir(output)
        for asset in assets:
            shutil.move(asset, output)
