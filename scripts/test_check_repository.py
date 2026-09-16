"""Regression tests: reject wiring drift while allowing preserved legacy sources."""

import contextlib
import io
import json
import unittest
from pathlib import Path
from unittest.mock import patch

import check_repository as checks


class RepositoryChecksTest(unittest.TestCase):
    def assert_rejected_text(self, path, replacement, check):
        original_read = Path.read_text

        def read(candidate, *args, **kwargs):
            if candidate == path:
                return replacement
            return original_read(candidate, *args, **kwargs)

        with patch.object(Path, "read_text", read):
            with contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit):
                    check()

    def test_current_and_legacy_profiles_pass(self):
        checks.validate_config()
        checks.validate_sketches()
        checks.validate_upstream_snapshot()

    def test_swapped_leg_label_is_rejected(self):
        config = json.loads(checks.CONFIG_PATH.read_text(encoding="utf-8"))
        config["servos"][3]["leg"] = "front_left"
        self.assert_rejected_text(checks.CONFIG_PATH, json.dumps(config), checks.validate_config)

    def test_wrong_side_group_is_rejected(self):
        path = checks.CONFIGURED_SKETCHES[0]
        source = path.read_text(encoding="utf-8").replace("{1, 7}", "{1, 4}")
        self.assert_rejected_text(path, source, checks.validate_sketches)

    def test_wrong_v3_joint_is_rejected(self):
        path = checks.CONFIGURED_SKETCHES[1]
        source = path.read_text(encoding="utf-8").replace(
            "kRightRearThigh = 4;", "kRightRearThigh = 7;")
        self.assert_rejected_text(path, source, checks.validate_sketches)

    def test_numeric_offset_comments_are_ignored(self):
        path = checks.CONFIGURED_SKETCHES[1]
        source = path.read_text(encoding="utf-8").replace("// left rear", "// channels 0-2")
        original_read = Path.read_text
        with patch.object(Path, "read_text", lambda p, *a, **kw:
                          source if p == path else original_read(p, *a, **kw)):
            checks.validate_sketches()

    def test_legacy_edit_is_rejected(self):
        original_read = Path.read_bytes
        with patch.object(Path, "read_bytes", lambda p:
                          original_read(p) + b"changed" if p.stem == "pitch_stabilization_v1"
                          else original_read(p)):
            with contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit):
                    checks.validate_sketches()


if __name__ == "__main__":
    unittest.main()
