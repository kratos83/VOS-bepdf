#!/bin/sh
# Usage: docker/build.sh <dir with the vos, vos-dev and vos-data debs> \
#                        [buildtools dir (default: ../Vitruvian/buildtools)] [out-dir (default: .)]
set -e
VOS_DEBS="$(realpath "${1:?directory with the vos debs}")"
cd "$(dirname "$0")/.."
BT="$(realpath "${2:-../Vitruvian/buildtools}")"
OUT="${3:-.}"

ls "$VOS_DEBS"/*.deb >/dev/null

# Host builds of the resource / catalog tools, in one flat directory.
TOOLS="$(mktemp -d)"
trap 'rm -rf "$TOOLS"' EXIT
cp "$BT/src/bin/rc/rc" "$BT/src/bin/xres" "$BT/src/bin/resattr" \
   "$BT/src/tools/locale/linkcatkeys" "$TOOLS/"

DOCKER_BUILDKIT=1 docker build \
    --build-context vos="$VOS_DEBS" \
    --build-context tools="$TOOLS" \
    --target deb --output "type=local,dest=$OUT" \
    -f docker/Dockerfile .
ls -la "$OUT"/*.deb
