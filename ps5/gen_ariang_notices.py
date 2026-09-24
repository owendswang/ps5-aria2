#!/usr/bin/env python3
import json
import sys
from pathlib import Path

source, ofl, target = map(Path, sys.argv[1:4])
manifest = json.loads((source / 'package.json').read_text())
lines = [
    '# AriaNg browser dependency notices\n\n',
    'These packages are included in the embedded AriaNg page. ',
    'AriaNg itself is covered by `AriaNg-LICENSE.md`.\n\n',
]
for name in sorted(manifest['dependencies']):
    package_dir = source / 'node_modules' / name
    package = json.loads((package_dir / 'package.json').read_text())
    license_files = sorted(path for path in package_dir.iterdir()
                           if path.is_file() and path.name.lower().startswith(
                               ('license', 'licence', 'copying')))
    license_name = package.get('license', package.get('licenses', 'See package notice'))
    lines.append(f'## {name} {package["version"]}\n\n')
    lines.append(f'Package license declaration: `{license_name}`.\n\n')
    if license_files:
        for license_file in license_files:
            lines.append(f'### {license_file.name}\n\n')
            lines.append('~~~text\n')
            lines.append(license_file.read_text(errors='replace').rstrip() + '\n')
            lines.append('~~~\n\n')
    else:
        lines.append('The package does not include a separate license file; '
                     'the declaration above comes from its package metadata.\n\n')
    if name == 'font-awesome':
        lines.append('Font Awesome 4.7.0 by Dave Gandy: its bundled font is '
                     'licensed under SIL OFL 1.1 and its CSS under MIT. '
                     'The package README identifies these terms.\n\n')
        lines.append('### SIL OFL 1.1\n\n~~~text\n')
        lines.append(ofl.read_text().rstrip() + '\n~~~\n\n')

target.parent.mkdir(parents=True, exist_ok=True)
target.write_text(''.join(lines))
