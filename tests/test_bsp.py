import io
import json
import struct
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path

from quakebsp import BSPError, BSPFile, LUMP_NAMES
from quakebsp.cli import main


def make_bsp(version=29, lumps=None):
    lump_data = lumps or {}
    offset = 124
    entries = []
    contents = []
    for name in LUMP_NAMES:
        data = lump_data.get(name, b"")
        entries.extend((offset if data else 0, len(data)))
        if data:
            contents.append(data)
            offset += len(data)
    return struct.pack("<i" + "ii" * 15, version, *entries) + b"".join(contents)


class BSPFileTests(unittest.TestCase):
    def test_reads_bsp29_lump_directory(self):
        data = make_bsp(lumps={"planes": bytes(40), "textures": b"texture data"})

        bsp = BSPFile.from_bytes(data)

        self.assertEqual(bsp.size, len(data))
        self.assertEqual(len(bsp.lumps), 15)
        planes = bsp.lumps[1]
        self.assertEqual((planes.offset, planes.length, planes.record_count), (124, 40, 2))
        self.assertEqual(bsp.lumps[2].record_count, None)

    def test_rejects_truncated_header(self):
        with self.assertRaisesRegex(BSPError, "too small"):
            BSPFile.from_bytes(bytes(123))

    def test_rejects_unsupported_version(self):
        with self.assertRaisesRegex(BSPError, "expected 29"):
            BSPFile.from_bytes(make_bsp(version=30))

    def test_rejects_lump_outside_file(self):
        data = bytearray(make_bsp())
        struct.pack_into("<ii", data, 4, 124, 4)
        with self.assertRaisesRegex(BSPError, "extends beyond"):
            BSPFile.from_bytes(data)

    def test_rejects_misaligned_fixed_record_lump(self):
        with self.assertRaisesRegex(BSPError, "multiple of its 20-byte"):
            BSPFile.from_bytes(make_bsp(lumps={"planes": bytes(21)}))

    def test_cli_inspects_valid_file_and_rejects_invalid_file(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "map.bsp"
            path.write_bytes(make_bsp(lumps={"planes": bytes(20)}))
            output = io.StringIO()
            with redirect_stdout(output):
                result = main(["inspect", str(path)])
            self.assertEqual(result, 0)
            self.assertIn("BSP version 29", output.getvalue())
            self.assertIn("planes", output.getvalue())

            output = io.StringIO()
            with redirect_stdout(output):
                result = main(["inspect", str(path), "--json"])
            self.assertEqual(result, 0)
            self.assertEqual(json.loads(output.getvalue())["lumps"][1]["records"], 1)

            path.write_bytes(b"bad")
            errors = io.StringIO()
            with redirect_stderr(errors):
                result = main(["inspect", str(path)])
            self.assertEqual(result, 1)
            self.assertIn("too small", errors.getvalue())


if __name__ == "__main__":
    unittest.main()
