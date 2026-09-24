# aria2 for PS5

This fork builds aria2 as a PS5 ELF payload. See the [upstream README](README.rst) and the [PS5 build guide](ps5/README.md).

## PS5 changes

| Area | Change |
| --- | --- |
| Build | `Makefile.ps5` uses the PS5 SDK, SDK SQLite3/libssh2, static c-ares, and static OpenSSL with its legacy provider built in. |
| Startup | The ELF uses `/data/aria2/aria2.conf` without elfldr arguments, creates `/data/aria2`, names the process `aria2.elf`, and allows one instance through `/data/aria2/aria2.lock`. A duplicate launch shows an already-running notification. |
| Configuration | Active entries in `ps5/aria2.conf.in` are embedded as defaults. An existing config file overrides them; explicit command-line options take precedence. |
| Interface | The RPC listener serves embedded AriaNg at `/` and `/index.html`; `/jsonrpc` remains the RPC endpoint. Startup installs a Media shortcut for `http://localhost:6800/` using AriaNg's icon and shows a PS5 notification. |

## Default configuration

| Setting | Value |
| --- | --- |
| RPC | Enabled on all interfaces, port `6800`, `/jsonrpc`, any origin allowed, maximum request size `2M`; no authentication or TLS. |
| Process | `daemon=false`; one instance per `/data/aria2/aria2.lock`. |
| Downloads | `dir=/data/aria2/downloads`, `continue=true`, `file-allocation=prealloc`, `auto-save-interval=60`. |
| Connections | 5 concurrent downloads, 5 connections per server, 10 splits with a minimum size of `10M`. |
| Speed limits | Download and upload limits are `0` (unlimited). |
| Session | Load and save `/data/aria2/aria2.session`; save every 10 seconds and on exit. The ELF creates an empty file if needed. |
| Console | `show-console-readout=false`, `console-log-level=notice`. |
| DHT state | `/data/aria2/dht.dat` and `/data/aria2/dht6.dat`. |
| Logging | `/data/aria2/aria2.log`, level `notice`. |
| Network and DNS | `disable-ipv6=true`, `async-dns=true`, `async-dns-server=192.168.1.1`. |
| HTTPS | `check-certificate=false`; CA file `/data/aria2/ca-bundle.crt`. The compile-time CA path is `/system/common/cert/CA_LIST.cer`. |
| Config file | `/data/aria2/aria2.conf` is optional; embedded defaults work without it. |

To verify HTTPS certificates, set `check-certificate=true` in the config file and provide a usable CA bundle. Change `async-dns-server` if `192.168.1.1` is unreachable.

## Build and run

```sh
make -f Makefile.ps5
nc PS5_IP 9021 < ps5-build/aria2.elf
```

| Output | Use |
| --- | --- |
| `ps5-build/aria2.elf` | Send directly to elfldr; no console-side ELF copy is needed. |
| `ps5-build/aria2/aria2.conf` | Optional: copy to `/data/aria2/aria2.conf` to change defaults. |
| `ps5-build/aria2/ca-bundle.crt` | Optional: copy to `/data/aria2/ca-bundle.crt` for certificate verification. |

AriaNg: `http://PS5_IP:6800/` · RPC: `http://PS5_IP:6800/jsonrpc` · PS5 Media shortcut: `AriaNg` (requires the payload to be running; fixed to port `6800`).

## Credits

| Project | Use |
| --- | --- |
| [aria2](https://github.com/aria2/aria2) | Upstream source. |
| [PS5 payload SDK](https://github.com/ps5-payload-dev/sdk) | Toolchain and PS5 libraries. |
| [elfldr](https://github.com/ps5-payload-dev/elfldr) | Payload loader and transfer protocol. |
| [ftpsrv](https://github.com/ps5-payload-dev/ftpsrv) | Reference for process naming and PS5 notifications. |
| [PS5 Web File Manager](https://github.com/owendswang/ps5-web-file-manager) | Reference for installing a PS5 Media web shortcut. |
| [AriaNg](https://github.com/mayswind/AriaNg) | Embedded web interface. |
| [c-ares](https://github.com/c-ares/c-ares) | Asynchronous DNS. |
| [OpenSSL](https://github.com/openssl/openssl) | TLS and cryptography. |
| [libssh2](https://github.com/libssh2/libssh2), [SQLite](https://sqlite.org/src/) | Libraries supplied by the PS5 SDK. |
| [curl CA Extract](https://curl.se/docs/caextract.html) | Mozilla CA bundle packaged with the build. |

## License

The aria2 source and PS5 modifications are distributed under GPL-2.0-or-later. Some upstream source files also carry an OpenSSL linking exception; see their headers and `LICENSE.OpenSSL`. Third-party components retain their own terms.

| Component | License or notice |
| --- | --- |
| aria2 and this fork | [GPL-2.0 text](COPYING), [OpenSSL exception](LICENSE.OpenSSL) |
| PS5 payload SDK | [SDK license](https://github.com/ps5-payload-dev/sdk/blob/master/LICENSE); [SDK README](https://github.com/ps5-payload-dev/sdk#license) states GPL-3.0-or-later for most files and BSD terms for `include/freebsd`. |
| elfldr, ftpsrv (references only) | [elfldr license](https://github.com/ps5-payload-dev/elfldr/blob/master/LICENSE), [ftpsrv license](https://github.com/ps5-payload-dev/ftpsrv/blob/master/LICENSE) |
| PS5 Web File Manager (reference only) | [GPL-3.0 license](https://github.com/owendswang/ps5-web-file-manager/blob/main/LICENSE) |
| AriaNg | [MIT license](ps5-licenses/AriaNg-LICENSE.md) |
| AriaNg browser dependencies | [Third-party notices](ps5-licenses/AriaNg-THIRD-PARTY-NOTICES.md) |
| c-ares | [MIT license](ps5-licenses/c-ares-LICENSE.md) |
| OpenSSL | [Apache-2.0 license](ps5-licenses/openssl-LICENSE.txt) |
| libssh2 | [libssh2 license](https://libssh2.org/license.html) |
| SQLite | [Public-domain statement](https://www.sqlite.org/copyright.html) |
| Mozilla CA bundle | [MPL-2.0](https://www.mozilla.org/en-US/MPL/2.0/), [bundle provenance](https://curl.se/docs/caextract.html) |

Keep `ps5-licenses/` and the applicable third-party notices when distributing the ELF. Build caches under `.ps5-work/` are generated and ignored by Git.
