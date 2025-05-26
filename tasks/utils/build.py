import subprocess
import logging

logger = logging.getLogger(__name__)


def build_project(board: str, target: str):
    """Build project"""
    logger.info(f"Building project ({board=})")

    # Clean build directory
    t = subprocess.run(
        ["make", "clean", f"TARGET={target}"],
        cwd="examples/osf-flock"
    )

    # Compile app
    t = subprocess.run(
        [
            "make", "-j16", "node", f"TARGET={target}", f"BOARD={board}",
            "DEPLOYMENT=nulltb", "PERIOD=1000", "CHN=1", "LOGGING=0", "GPIO=0", "LEDS=1", "NTX=6", "NSLOTS=6", "PWR=ZerodBm", "PROTO=OSF_PROTO_STT"
        ],
        cwd="examples/osf-flock"
    )

    if t.returncode != 0:
        raise Exception(f"Failed to build project for board {board}")

    # Show build output
    t = subprocess.run(
        ["tree", "-L", "4"],
        cwd="examples/osf-flock",
        capture_output=False
    )

    # Generate hex file from .elf
    t = subprocess.run(
        ["arm-none-eabi-objcopy", "-O", "ihex", f"build/nrf/{board}/node.elf", f"build/nrf/{board}/node.hex"],
        cwd="examples/osf-flock",
    )

    if t.returncode != 0:
        raise Exception(f"Failed to generate hex file for board {board}")
