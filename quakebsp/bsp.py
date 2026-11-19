import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Tuple

VERSION = 29
LUMP_NAMES = (
    "entities",
    "planes",
    "textures",
    "vertices",
    "visibility",
    "nodes",
    "texinfo",
    "faces",
    "lighting",
    "clipnodes",
    "leaves",
    "marksurfaces",
    "edges",
    "surfedges",
    "models",
)
RECORD_SIZES = {
    "planes": 20,
    "vertices": 12,
    "nodes": 24,
    "texinfo": 40,
    "faces": 20,
    "clipnodes": 8,
    "leaves": 28,
    "marksurfaces": 2,
    "edges": 4,
    "surfedges": 4,
    "models": 64,
}
HEADER_SIZE = 4 + len(LUMP_NAMES) * 8
_HEADER = struct.Struct("<i" + "ii" * len(LUMP_NAMES))


class BSPError(ValueError):
    """Raised when a file is not a valid original Quake BSP."""


@dataclass(frozen=True)
class Lump:
    name: str
    offset: int
    length: int

    @property
    def record_size(self):
        return RECORD_SIZES.get(self.name)

    @property
    def record_count(self):
        if self.record_size is None:
            return None
        return self.length // self.record_size


@dataclass(frozen=True)
class BSPFile:
    lumps: Tuple[Lump, ...]
    size: int

    @classmethod
    def from_bytes(cls, data):
        if len(data) < HEADER_SIZE:
            raise BSPError(
                f"file is too small for a BSP29 header: expected at least "
                f"{HEADER_SIZE} bytes, got {len(data)}"
            )

        header = _HEADER.unpack_from(data)
        version = header[0]
        if version != VERSION:
            raise BSPError(f"unsupported BSP version {version}; expected {VERSION}")

        lumps = []
        for index, name in enumerate(LUMP_NAMES):
            offset, length = header[1 + index * 2 : 3 + index * 2]
            if offset < 0 or length < 0:
                raise BSPError(f"{name} lump has a negative offset or length")
            if length and offset < HEADER_SIZE:
                raise BSPError(f"{name} lump overlaps the BSP header")
            if offset > len(data) or length > len(data) - offset:
                raise BSPError(f"{name} lump extends beyond the end of the file")
            record_size = RECORD_SIZES.get(name)
            if record_size is not None and length % record_size:
                raise BSPError(
                    f"{name} lump length {length} is not a multiple of "
                    f"its {record_size}-byte record size"
                )
            lumps.append(Lump(name, offset, length))

        return cls(tuple(lumps), len(data))

    @classmethod
    def read(cls, path):
        return cls.from_bytes(Path(path).read_bytes())
