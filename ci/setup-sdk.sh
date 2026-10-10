#!/bin/sh
# Turn a Debian trixie system into a V\OS build host without installing V\OS.
#
#   ci/setup-sdk.sh <dir with the vos, vos-dev and vos-data debs>
#
# The debs are unpacked, not installed, so none of their postinst scripts
# (systemd units, the /system layout, grub...) ever runs. They are told apart
# by their Package field, so repo names (vos_<ver>_amd64.deb) and local build
# names (VOS-<ver>-Linux-runtime.deb) both work. Needs root.
set -e

DEBS="${1:?directory with the vos, vos-dev and vos-data debs}"
export DEBIAN_FRONTEND=noninteractive

runtime=""
for f in "$DEBS"/*.deb; do
    case "$(dpkg-deb -f "$f" Package)" in
        vos)      runtime="$f" ;;
        vos-dev)  dev="$f" ;;
        vos-data) data="$f" ;;
    esac
done
for v in "$runtime" "$dev" "$data"; do
    [ -f "$v" ] || { echo "setup-sdk: need vos, vos-dev and vos-data in $DEBS" >&2; exit 1; }
done

apt-get update
# Runtime libraries libbe.so & co. link against, from the vos package's own
# Depends (fonts, bluez, gstreamer plugins... are not needed to link).
libs=$(dpkg-deb -f "$runtime" Depends \
    | tr ',' '\n' | sed 's/([^)]*)//g; s/|.*//; s/ //g' \
    | grep '^lib' | sort -u)
apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build pkg-config attr dpkg-dev \
    libfreetype-dev libicu-dev libnm0 libsystemd0 \
    libgstreamer-plugins-base1.0-0 $libs

# /bin and /lib are symlinks on trixie: keep them.
for d in "$runtime" "$dev" "$data"; do
    dpkg-deb --fsys-tarfile "$d" | tar -C / --keep-directory-symlink -xf -
done
ldconfig

echo "--- unresolved shared library dependencies (none expected):"
for f in /lib/libbe.so /lib/libmedia2.so /lib/libtracker.so /lib/libgame.so; do
    ldd "$f" | grep 'not found' || true
done
