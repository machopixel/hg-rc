#!/usr/bin/env python3
"""List, verify and extract Heavy Gear's HEAVYG.PRJ asset archive.

HEAVYG.MAP is the index, HEAVYG.PRJ holds the data. See docs/formats/prj.md.

    python prjtool.py list    <game dir> [--type WAV]
    python prjtool.py verify  <game dir>
    python prjtool.py extract <game dir> <out dir> [--type WAV]

Extracted files land in <out dir>/<group letter>/<type>/<name>.<type>. A name
that repeats within a group gets ~2, ~3... appended in archive order.
"""
import argparse
import collections
import os
import struct
import sys
import zlib

from blast import explode

COMP_NONE, COMP_PKWARE, COMP_ZLIB = 0, 1, 2
ENTRY_HEADER = struct.Struct("<12sHII")  # tag, compression, size, packed size


class Entry:
    __slots__ = ("name", "ext", "offset", "group", "comp", "size", "packed")

    @property
    def filename(self):
        return f"{self.name}.{self.ext}"


def _cstr(raw):
    return raw.split(b"\0", 1)[0].decode("latin-1")


def read_index(game_dir):
    with open(os.path.join(game_dir, "HEAVYG.MAP"), "rb") as f:
        m = f.read()
    if m[:2] != b"YL":
        raise ValueError("HEAVYG.MAP: bad magic")
    ntypes, = struct.unpack_from("<I", m, 4)
    pos = 8
    types = []
    for _ in range(ntypes):
        ext, count = struct.unpack_from("<4sI", m, pos)
        types.append((_cstr(ext), count))
        pos += 8
    _, ncomp = struct.unpack_from("<II", m, pos)
    pos += 8 + 16 * ncomp  # compressor DLL names: pkware.dll, zlib.dll
    entries = []
    while pos + 20 <= len(m):
        name, ext, offset, _ = struct.unpack_from("<8s4sI4s", m, pos)
        e = Entry()
        e.name, e.ext, e.offset = _cstr(name), _cstr(ext), offset
        entries.append(e)
        pos += 20
    if len(entries) != sum(c for _, c in types):
        raise ValueError("HEAVYG.MAP: entry count does not match type table")
    return types, entries


def open_archive(game_dir):
    types, entries = read_index(game_dir)
    prj = open(os.path.join(game_dir, "HEAVYG.PRJ"), "rb")
    for e in entries:
        prj.seek(e.offset)
        tag, e.comp, e.size, e.packed = ENTRY_HEADER.unpack(prj.read(ENTRY_HEADER.size))
        e.group = tag[7:8].decode("latin-1")  # tag is "C:\PRJ\" + group letter + type
    return types, entries, prj


def read_entry(prj, e):
    prj.seek(e.offset + ENTRY_HEADER.size)
    data = prj.read(e.packed)
    if e.comp == COMP_NONE:
        out = data
    elif e.comp == COMP_PKWARE:
        out = explode(data)
    elif e.comp == COMP_ZLIB:
        out = zlib.decompress(data)
    else:
        raise ValueError(f"{e.filename}: unknown compression {e.comp}")
    if len(out) != e.size:
        raise ValueError(f"{e.filename}: expected {e.size} bytes, got {len(out)}")
    return out


def _select(entries, ext):
    return [e for e in entries if ext is None or e.ext.upper() == ext.upper()]


def cmd_list(args):
    _, entries, _ = open_archive(args.game_dir)
    for e in _select(entries, args.type):
        print(f"{e.filename:<14} group {e.group}  comp {e.comp}  {e.size:>9}  @{e.offset:#010x}")


def cmd_verify(args):
    types, entries, prj = open_archive(args.game_dir)
    end = os.fstat(prj.fileno()).st_size
    starts = sorted(e.offset for e in entries) + [end]
    gaps = sum(1 for e, nxt in zip(sorted(entries, key=lambda e: e.offset), starts[1:])
               if e.offset + ENTRY_HEADER.size + e.packed != nxt)
    failed = 0
    for e in entries:
        try:
            read_entry(prj, e)
        except Exception as exc:  # report every bad entry, keep going
            failed += 1
            print(f"FAIL {e.filename}: {exc}", file=sys.stderr)
    comps = collections.Counter(e.comp for e in entries)
    print(f"{len(entries)} entries in {len(types)} types, {end} bytes")
    print(f"compression: none {comps[0]}, pkware {comps[1]}, zlib {comps[2]}")
    print(f"entries not tiling the archive: {gaps}; entries failing to decode: {failed}")
    return 1 if failed or gaps else 0


def cmd_extract(args):
    _, entries, prj = open_archive(args.game_dir)
    seen = collections.Counter()
    for e in _select(entries, args.type):
        folder = os.path.join(args.out_dir, e.group, e.ext)
        os.makedirs(folder, exist_ok=True)
        key = (e.group, e.filename.upper())
        seen[key] += 1
        # A few names repeat within a group with different data; keep every copy.
        name = e.filename if seen[key] == 1 else f"{e.name}~{seen[key]}.{e.ext}"
        with open(os.path.join(folder, name), "wb") as f:
            f.write(read_entry(prj, e))
    print(f"extracted {sum(seen.values())} entries to {args.out_dir}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("list"); p.add_argument("game_dir"); p.add_argument("--type")
    p.set_defaults(fn=cmd_list)
    p = sub.add_parser("verify"); p.add_argument("game_dir")
    p.set_defaults(fn=cmd_verify)
    p = sub.add_parser("extract"); p.add_argument("game_dir"); p.add_argument("out_dir"); p.add_argument("--type")
    p.set_defaults(fn=cmd_extract)
    args = ap.parse_args()
    return args.fn(args) or 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BrokenPipeError:  # output piped into head/more
        sys.exit(0)
