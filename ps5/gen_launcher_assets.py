#!/usr/bin/env python3
import json
import sys
from pathlib import Path

param, icon, target = map(Path, sys.argv[1:4])
metadata = json.loads(param.read_text())
if metadata['titleId'] != 'ARIA26800' or metadata['deeplinkUri'] != 'http://localhost:6800/':
    raise SystemExit('Unexpected PS5 launcher metadata')


def literal(data):
    return ''.join(
        chr(byte) if 32 <= byte <= 126 and byte not in (34, 63, 92)
        else f'\\{byte:03o}' for byte in data
    )


lines = ['#pragma once\n']
for symbol, source in (('kPs5LauncherParam', param), ('kPs5LauncherIcon', icon)):
    payload = source.read_bytes()
    if not payload:
        raise SystemExit(f'empty launcher asset: {source}')
    lines.append(f'static const char {symbol}[] =\n')
    for offset in range(0, len(payload), 4096):
        lines.append(f'    "{literal(payload[offset:offset + 4096])}"\n')
    lines.append('    ;\n')
target.parent.mkdir(parents=True, exist_ok=True)
temporary = target.with_name(target.name + '.tmp')
temporary.write_text(''.join(lines))
temporary.replace(target)
