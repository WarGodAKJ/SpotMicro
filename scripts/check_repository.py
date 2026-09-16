#!/usr/bin/env python3
"""Validate repository layout, hardware configuration, and source provenance."""

from __future__ import annotations

import hashlib
import json
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CONFIG_PATH = ROOT / "config" / "robot.json"
EXPECTED_OFFSETS = [-3, 7, -4, 3, -1, 3, -3, -2, -2, 2, 4, 7]
EXPECTED_LEGS = ["rear_left", "rear_right", "front_left", "front_right"]
LEGACY_BLOBS = {
    "pitch_stabilization_v1": "9b9cc7ad9dcf335a05ab8e3a97b54634ca4e5161",
    "pitch_stabilization_v2": "3d076b67094edc97c1916541453269bcc5e61be4",
}
CONFIGURED_SKETCHES = [
    ROOT / "firmware" / "demos" / "push_ups" / "push_ups.ino",
    ROOT / "firmware" / "experiments" / "pitch_roll_stabilization_v3" / "pitch_roll_stabilization_v3.ino",
]
UPSTREAM_BLOBS = {
    "AsyncServo.h": "bf79b4bf67d9062ab4b8126dcd862dd935b6d665",
    "MPU6050_conf.h": "e319a723e22078678247bc992a44c47248e95cad",
    "NovaServos.h": "54aac68cea8dbadbf858afd29fcd77b97fe8404d",
    "nova_sm3_teensy_v4_2.ino": "f04de9a6602792bb3137cbe691d8a8f89911f00e",
}
REQUIRED_FILES = [
    ROOT / "README.md",
    ROOT / "CHANGELOG.md",
    ROOT / "CONTRIBUTING.md",
    ROOT / "SECURITY.md",
    ROOT / "THIRD_PARTY_NOTICES.md",
    ROOT / "docs" / "architecture.md",
    ROOT / "docs" / "hardware.md",
    ROOT / "docs" / "calibration.md",
    ROOT / "docs" / "testing.md",
    ROOT / "docs" / "roadmap.md",
]


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def git_blob_sha(data: bytes) -> str:
    normalized = data.replace(b"\r\n", b"\n")
    payload = b"blob " + str(len(normalized)).encode("ascii") + b"\0" + normalized
    return hashlib.sha1(payload).hexdigest()


def validate_config() -> None:
    config = json.loads(CONFIG_PATH.read_text(encoding="utf-8"))
    servos = config["servos"]
    channels = [servo["channel"] for servo in servos]
    if channels != list(range(12)):
        fail(f"servo channels must be exactly 0-11, found {channels}")

    offsets = [servo["offset_deg"] for servo in servos]
    if offsets != EXPECTED_OFFSETS:
        fail(f"robot.json offsets do not match the recorded calibration: {offsets}")

    for channel, servo in enumerate(servos):
        if servo["leg"] != EXPECTED_LEGS[channel // 3]:
            fail(f"wrong leg at channel {channel}: {servo['leg']}")
        if servo["joint"] != ["hip", "thigh", "knee"][channel % 3]:
            fail(f"wrong joint at channel {channel}: {servo['joint']}")
        expected_class = "35kg" if servo["joint"] == "knee" else "20kg"
        if servo["servo_class"] != expected_class:
            fail(
                f"channel {servo['channel']} is {servo['joint']} but uses "
                f"servo class {servo['servo_class']}"
            )


def validate_sketches() -> None:
    all_sketches = sorted((ROOT / "firmware").rglob("*.ino"))
    if not all_sketches:
        fail("no Arduino sketches found")

    for sketch in all_sketches:
        if sketch.parent.name != sketch.stem:
            fail(f"Arduino sketch directory must match filename: {sketch.relative_to(ROOT)}")
        if "[cite:" in sketch.read_text(encoding="utf-8", errors="replace"):
            fail(f"export citation marker found in {sketch.relative_to(ROOT)}")

    offsets_pattern = re.compile(
        r"kServoOffsets\s*\[[^\]]+\]\s*=\s*\{(?P<body>[^}]+)\}", re.DOTALL
    )
    for sketch in CONFIGURED_SKETCHES:
        source = sketch.read_text(encoding="utf-8")
        match = offsets_pattern.search(source)
        if match is None:
            fail(f"missing kServoOffsets array in {sketch.relative_to(ROOT)}")
        body = re.sub(r"//[^\n]*", "", match.group("body"))
        offsets = [int(value) for value in re.findall(r"-?\d+", body)]
        if offsets != EXPECTED_OFFSETS:
            fail(f"offset mismatch in {sketch.relative_to(ROOT)}: {offsets}")

    demo = CONFIGURED_SKETCHES[0].read_text(encoding="utf-8")
    for name, expected in {
        "kLeftThighs": [1, 7], "kLeftKnees": [2, 8],
        "kRightThighs": [4, 10], "kRightKnees": [5, 11],
    }.items():
        match = re.search(rf"{name}\[\]\s*=\s*\{{([^}}]+)\}}", demo)
        if not match or [int(x) for x in re.findall(r"\d+", match[1])] != expected:
            fail(f"wrong active push-up group: {name}")

    active = CONFIGURED_SKETCHES[1].read_text(encoding="utf-8")
    for leg_index, leg in enumerate(["LeftRear", "RightRear", "FrontLeft", "FrontRight"]):
        for joint_index, joint in enumerate(["Hip", "Thigh", "Knee"]):
            channel = 3 * leg_index + joint_index
            if not re.search(rf"k{leg}{joint}\s*=\s*{channel}\s*;", active):
                fail(f"wrong v3 channel: {leg}{joint}")

    config = json.loads(CONFIG_PATH.read_text(encoding="utf-8"))
    for name, expected in {
        "kPwmAddress": int(config["servo_driver"]["i2c_address"], 16),
        "kMpuAddress": int(config["imu"]["i2c_address"], 16),
        "kMpuSdaPin": config["imu"]["sda_pin"],
        "kMpuSclPin": config["imu"]["scl_pin"],
        "kServoFrequencyHz": config["servo_driver"]["frequency_hz"],
        "kOscillatorFrequencyHz": config["servo_driver"]["oscillator_hz"],
    }.items():
        match = re.search(rf"{name}\s*=\s*(0x[0-9a-fA-F]+|\d+)\s*;", active)
        if not match or int(match[1], 0) != expected:
            fail(f"v3/config mismatch: {name}")
    if f'Serial.begin({config["controller"]["serial_baud"]})' not in active:
        fail("v3 serial baud differs from config")
    if f'Wire1.setClock({config["imu"]["clock_hz"]})' not in active:
        fail("v3 IMU clock differs from config")

    for name, expected_sha in LEGACY_BLOBS.items():
        path = ROOT / "firmware" / "experiments" / name / f"{name}.ino"
        if git_blob_sha(path.read_bytes()) != expected_sha:
            fail(f"legacy milestone source changed: {name}")


def validate_upstream_snapshot() -> None:
    directory = ROOT / "firmware" / "reference" / "nova_sm3_teensy_v4_2"
    for name, expected_sha in UPSTREAM_BLOBS.items():
        path = directory / name
        actual_sha = git_blob_sha(path.read_bytes())
        if actual_sha != expected_sha:
            fail(
                f"upstream snapshot changed: {path.relative_to(ROOT)} "
                f"has {actual_sha}, expected {expected_sha}"
            )


def validate_markdown_links() -> None:
    link_pattern = re.compile(r"\[[^\]]+\]\(([^)]+)\)")
    for markdown in ROOT.rglob("*.md"):
        text = markdown.read_text(encoding="utf-8")
        for raw_target in link_pattern.findall(text):
            target = raw_target.strip().split("#", 1)[0]
            if not target or "://" in target or target.startswith("mailto:"):
                continue
            resolved = (markdown.parent / target).resolve()
            if not resolved.exists():
                fail(f"broken local link in {markdown.relative_to(ROOT)}: {raw_target}")


def main() -> None:
    missing = [path.relative_to(ROOT) for path in REQUIRED_FILES if not path.is_file()]
    if missing:
        fail(f"required files missing: {', '.join(map(str, missing))}")

    validate_config()
    validate_sketches()
    validate_upstream_snapshot()
    validate_markdown_links()
    print("Repository checks passed.")


if __name__ == "__main__":
    main()
