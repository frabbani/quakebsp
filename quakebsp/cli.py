import argparse
import json
import sys

from .bsp import BSPError, BSPFile


def _lump_summary(lump):
    return {
        "name": lump.name,
        "offset": lump.offset,
        "length": lump.length,
        "records": lump.record_count,
    }


def main(argv=None):
    parser = argparse.ArgumentParser(
        prog="quakebsp",
        description="Inspect and validate original Quake BSP29 files.",
    )
    subparsers = parser.add_subparsers(dest="command", required=True)
    inspect_parser = subparsers.add_parser("inspect", help="validate and list BSP lumps")
    inspect_parser.add_argument("file", help="path to a Quake BSP29 file")
    inspect_parser.add_argument(
        "--json", action="store_true", help="print the lump table as JSON"
    )
    args = parser.parse_args(argv)

    try:
        bsp = BSPFile.read(args.file)
    except (OSError, BSPError) as error:
        print(f"quakebsp: {error}", file=sys.stderr)
        return 1

    if args.json:
        print(json.dumps({"version": 29, "size": bsp.size, "lumps": [
            _lump_summary(lump) for lump in bsp.lumps
        ]}, indent=2))
    else:
        print(f"BSP version 29 — {bsp.size} bytes")
        print(f"{'Lump':<14} {'Offset':>10} {'Length':>10} {'Records':>10}")
        for lump in bsp.lumps:
            records = "-" if lump.record_count is None else str(lump.record_count)
            print(f"{lump.name:<14} {lump.offset:>10} {lump.length:>10} {records:>10}")
    return 0
