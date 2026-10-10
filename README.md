# BePDF for V\OS

BePDF 2.1.4, a PDF reader based on xpdf 4.0, ported to V\OS (Vitruvian) and
packaged as `vos-bepdf`. It lives outside the Vitruvian tree: it builds
against an installed `vos-dev` and installs into `/system` like any other
V\OS application.

Upstream: <https://github.com/HaikuArchives/BePDF>. Licensed GPL; xpdf is
GPL v2/v3 (see `xpdf/COPYING`, `xpdf/COPYING3`, `data/license/xpdf`).

## Layout

| Path | What |
|---|---|
| `src/` | the application, `BePDF.rdef`, `utils/` |
| `xpdf/` | xpdf 4.0 with the BePDF patches, built as a static library |
| `locales/` | `.catkeys`, signature `x-vnd.mp-xpdf` |
| `data/` | fonts, encodings, licenses and `docs/Start.pdf`, installed in `/system/data/bepdf` |
| `packaging/` | `postinst` and `postrm` of the package |
| `cmake/Packaging.cmake` | install rules and CPack (DEB) |

## Build

On a V\OS system with `vos` and `vos-dev` installed (they provide `vos.pc`,
the public and private headers, `rc`, `xres`, `resattr` and `linkcatkeys`):

```sh
sudo apt install cmake ninja-build pkg-config libfreetype-dev libicu-dev
cmake -S . -B build -G Ninja
ninja -C build
ninja -C build package            # build/vos-bepdf_*.deb
sudo apt install ./build/vos-bepdf_*.deb
```

Private headers are read from `/system/develop/headers/private` (installed by
`vos`, or by `vos-dev` in newer builds); point
`-DVOS_PRIVATE_HEADERS=<dir>` somewhere else if vos-dev is not installed there.

## Build in Docker (no V\OS install needed)

`docker/Dockerfile` unpacks the `vos`, `vos-dev` and `vos-data` debs of a
Vitruvian build (no `postinst` runs), installs the libraries `libbe.so`
links against, and builds `vos-bepdf`:

```sh
docker/build.sh <dir with VOS-*-Linux-{runtime,dev,data}.deb> [buildtools-dir] [out-dir]
# e.g. docker/build.sh ~/Download/Vitruvian/generated.amd64 ~/Download/Vitruvian/buildtools
# -> ./vos-bepdf_2.1.4_amd64.deb
```

`rc`, `xres`, `resattr` and `linkcatkeys` from the `vos` package link libbe
and need Nexus IPC, so inside the container the host builds from Vitruvian's
`buildtools/` are used (`src/bin/rc/rc`, `src/bin/xres`, `src/bin/resattr`,
`src/tools/locale/linkcatkeys`).

## GitHub Actions

`.github/workflows/build-deb.yml` builds the deb for **amd64** and **arm64**
(one native runner each: `ubuntu-latest` and `ubuntu-24.04-arm`) on every push
and pull request, uploads them as the `vos-bepdf-deb-amd64` and
`vos-bepdf-deb-arm64` artifacts, and, when a `v*` tag is pushed, publishes one
release with both debs once both builds have succeeded:

```sh
git tag v2.1.4 && git push origin v2.1.4
```

The job runs in `debian:trixie`:

1. `ci/fetch-sdk-apt.sh`: adds `https://repo.v-os.dev` (key fingerprint
   pinned), suite `trixie-nightly`, and downloads `vos`, `vos-dev` and
   `vos-data`, checking each file against the SHA256 of the signed index (the
   amd64 files are published under `pool/pool/`, which the script knows);
2. unpacks them with `ci/setup-sdk.sh` (the same script the Dockerfile uses; no
   `postinst` runs);
3. builds `rc`, `xres`, `resattr` and `linkcatkeys` from a shallow clone of
   VitruvianOS/Vitruvian (`cmake -DBUILDTOOLS_MODE=1`, cached per architecture);
4. `cmake`, `ninja`, `ninja package`.

The suite and the Vitruvian ref are `workflow_dispatch` inputs, or the
repository variables `VOS_REPO_SUITE` and `VITRUVIAN_REF`.

## What the package does

- `/system/apps/BePDF`, data in `/system/data/bepdf`, catalogs in
  `/system/data/locale/catalogs/x-vnd.mp-xpdf`.
- `postinst` links BePDF into the Deskbar Applications menu and restores the
  BeOS attributes (signature, supported types, icon) from
  `/usr/share/vos-bepdf/attrs.tar`, since dpkg does not carry xattrs.
- `application/pdf` already points at `application/x-vnd.mp-xpdf` in the
  V\OS MIME database, so PDFs open with BePDF once it is installed.

## Notes on the port

- xpdf is built with `__HAIKU__` (atomics, Nimbus font table, public
  parameter tables), which the V\OS toolchain does not define by itself.
- Data is looked up in `/system/data/bepdf`; if that directory does not
  exist, the directory of the executable is used (running from a build tree).
- `Help` looks for `docs/English.pdf`, `Italiano.pdf`, ... which upstream
  downloaded in `build.sh`. Only `Start.pdf` is shipped.
