#!/usr/bin/env python3
import json
import sys
from pathlib import Path

source = Path(sys.argv[1])
target = Path(sys.argv[2])
entries = [
    line for line in source.read_text().splitlines()
    if line.strip() and not line.lstrip().startswith('#')
]
if any('=' not in line for line in entries):
    raise SystemExit('PS5 configuration contains an active line without =')
content = '\n'.join(entries) + '\n'
header = (
    '#pragma once\n'
    'static constexpr char kPs5Aria2Defaults[] = '
    + json.dumps(content, ensure_ascii=True) + ';\n'
)
target.parent.mkdir(parents=True, exist_ok=True)
temporary = target.with_name(target.name + '.tmp')
temporary.write_text(header)
temporary.replace(target)
