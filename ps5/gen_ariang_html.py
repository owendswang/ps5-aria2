#!/usr/bin/env python3
import sys
from pathlib import Path

source, target = map(Path, sys.argv[1:3])
html = source.read_bytes()
if not html:
    raise SystemExit(f"empty AriaNg bundle: {source}")


def literal(data):
    return ''.join(
        chr(byte) if 32 <= byte <= 126 and byte not in (34, 63, 92)
        else f'\\{byte:03o}' for byte in data
    )


lines = ['static const char kPs5AriaNgHtml[] =\n']
for offset in range(0, len(html), 4096):
    lines.append(f'    "{literal(html[offset:offset + 4096])}"\n')
lines.append('    ;\n')
target.parent.mkdir(parents=True, exist_ok=True)
target.write_text(''.join(lines))
