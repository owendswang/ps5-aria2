# PS5 aria2 payload

Build from the repository root with `make -f Makefile.ps5`. The build uses
`/opt/ps5-payload-sdk` by default; set `PS5_SDK=/another/path` if needed.

| Requirement | Tools |
| --- | --- |
| SDK | PS5 payload SDK with SQLite3 and libssh2. |
| Host build tools | GNU Make 4.3+, Autoconf, Automake, Libtool, CMake, Perl, Python 3, Node.js with npm, Git, curl, tar, sha256sum, and the OpenSSL command-line tool. |

The first build downloads c-ares 1.34.8 and OpenSSL 3.5.8 from their
official releases, plus the current Mozilla CA bundle from curl's CA service.
It downloads AriaNg 1.3.15 at pinned commit `d3ccb51` into `.ps5-work`,
installs npm dependencies, and builds AriaNg as one HTML page. Set `ARIANG_SRC` to use a
local source tree instead. `make -f Makefile.ps5 refresh-ca`
updates only the packaged CA bundle. Downloaded sources and intermediate files
stay in the ignored `.ps5-work` directory. `make` creates the output
directories when they do not exist.

OpenSSL is cross-compiled as static libraries with its legacy provider built
into `libcrypto`. The ELF includes the provider needed for BitTorrent's RC4
encrypted handshake and does not need an `ossl-modules` directory on the PS5.

The embedded settings disable HTTPS certificate verification by default.
The configuration file is optional; its active entries are embedded in
`aria2.elf` during the build. If the file exists, its settings override
those embedded defaults. The packaged CA bundle is useful if you set
`check-certificate=true`.

The deployable output contains only these files:

```text
ps5-build/
├── aria2.elf
└── aria2/
    ├── aria2.conf
    └── ca-bundle.crt
```

Copy `ps5-build/aria2/` to `/data/aria2/` on the PS5 if you want the
optional configuration and CA bundle. Send `aria2.elf` directly to elfldr;
it does not need to be stored on the PS5. The ELF creates `/data/aria2`
at startup if needed. aria2 creates the configured download directory when
it opens the first download file; it must be able to write there. Open
`http://PS5_IP:6800/` for the embedded AriaNg page. On each successful RPC startup,
the ELF writes any missing launcher files under `/user/app/ARIA26800/sce_sys/`
and asks PS5 to install the `AriaNg` Media shortcut. Its icon comes from AriaNg,
and its URL is fixed to `http://localhost:6800/`; the payload must be running
when the shortcut is opened. The same port serves
`/jsonrpc`; no separate web server or deployed HTML file is needed. The
embedded settings enable RPC on `0.0.0.0:6800/jsonrpc` without authentication or TLS,
set the CA and DNS paths, and configure logging and downloads as shown in
`ps5-build/aria2/aria2.conf`.

Send the raw ELF to an elfldr listening on port 9021:

```sh
nc PS5_IP 9021 < ps5-build/aria2.elf
```

The PS5 ELF checks `/data/aria2/aria2.conf` internally and uses the embedded
settings when that file is absent, so elfldr needs no arguments. `nc` stays
connected while aria2 runs; closing `nc` after RPC starts leaves the PS5
process running. An explicit `--conf-path` can still select another file for
testing. The payload names its process `aria2.elf` and sends a PS5 notification after RPC binds, showing the aria2
version and configured RPC port. `daemon=false` keeps aria2 in the payload
process started by elfldr. A second launch exits while the first instance
holds `/data/aria2/aria2.lock` and shows an already-running notification; the lock is released when that process exits.
The ELF creates an empty `/data/aria2/aria2.session` if needed. It loads that
file at startup, saves unfinished downloads there every 10 seconds and on exit,
and disables the periodic console readout that prints blank lines through `nc`.
The embedded settings select `/data/aria2/ca-bundle.crt` if HTTPS certificate
verification is enabled. The build also sets aria2's compile-time CA path to the PS5 system bundle
`/system/common/cert/CA_LIST.cer`.

The embedded settings disable IPv6 because the inspected PS5 does not support
IPv6 socket binding. With `async-dns=true`, this c-ares build reads `/etc/resolv.conf` for DNS
servers. That file was absent in the inspected PS5 filesystem, so the
embedded settings use `async-dns-server=192.168.1.1`, the server you
provided. Change the address if the PS5 cannot reach that DNS server.

The optional config includes the embedded PS5 settings and a commented index
of all option names defined by this aria2 source release. Leave commented
keys at their upstream defaults, or set valid values for the features you use.

The c-ares source archive and other build caches are generated in `.ps5-work`
and are not committed. Commit `ps5-licenses/` with a source fork and include
those notices when distributing the ELF. The c-ares MIT license is in
`ps5-licenses/c-ares-LICENSE.md`.
The OpenSSL Apache 2.0 license is in `ps5-licenses/openssl-LICENSE.txt`.
The AriaNg MIT license is copied to `ps5-licenses/AriaNg-LICENSE.md`.
Notices for the browser dependencies, including the bundled Font Awesome
font license, are in `ps5-licenses/AriaNg-THIRD-PARTY-NOTICES.md`.
