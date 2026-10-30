#!/usr/bin/env python3
"""Extract the icon groups from the original HG.EXE as .ico files.

    python extract_icons.py C:\\Path\\To\\HeavyGear\\HG.EXE build\\icons

Writes icon103.ico, icon104.ico, icon107.ico and icon108.ico. Point the
launcher build at that folder with -DHG_ICON_DIR=... Needs: pip install pefile
"""
import os
import struct
import sys

import pefile

RT_ICON, RT_GROUP_ICON = 3, 14


def resources(pe, type_id):
    for rtype in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if rtype.id != type_id:
            continue
        for entry in rtype.directory.entries:
            lang = entry.directory.entries[0]
            rva, size = lang.data.struct.OffsetToData, lang.data.struct.Size
            yield entry.id, pe.get_memory_mapped_image()[rva:rva + size]


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    pe = pefile.PE(sys.argv[1])
    icons = dict(resources(pe, RT_ICON))
    os.makedirs(sys.argv[2], exist_ok=True)
    for group_id, group in resources(pe, RT_GROUP_ICON):
        _, _, count = struct.unpack_from("<HHH", group, 0)
        header = struct.pack("<HHH", 0, 1, count)
        dirs, images = b"", b""
        offset = 6 + 16 * count
        for i in range(count):
            # GRPICONDIRENTRY is ICONDIRENTRY with a 2-byte resource id
            # in place of the 4-byte file offset.
            entry = group[6 + 14 * i: 6 + 14 * (i + 1)]
            icon_id, = struct.unpack_from("<H", entry, 12)
            image = icons[icon_id]
            dirs += entry[:8] + struct.pack("<II", len(image), offset)
            images += image
            offset += len(image)
        path = os.path.join(sys.argv[2], f"icon{group_id}.ico")
        with open(path, "wb") as f:
            f.write(header + dirs + images)
        print(path)


if __name__ == "__main__":
    main()
