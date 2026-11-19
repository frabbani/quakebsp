# quakebsp

`quakebsp` is a small, dependency-free command-line tool for inspecting and
validating original Quake (BSP version 29) map files.

Run it with Python 3.7 or later:

```sh
python -m quakebsp inspect path/to/map.bsp
```

The command checks the BSP version, lump bounds, and fixed-size lump record
alignment, then prints each lump's offset and size. Add `--json` for machine-
readable output:

```sh
python -m quakebsp inspect path/to/map.bsp --json
```

The parser is also available as `quakebsp.BSPFile`; load bytes with
`BSPFile.from_bytes(data)` or a file with `BSPFile.read(path)`. Invalid or
unsupported files raise `quakebsp.BSPError`.

Run the tests with:

```sh
python -m unittest discover -s tests
```
