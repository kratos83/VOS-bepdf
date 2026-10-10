#!/bin/sh
# Download vos, vos-dev and vos-data from the V\OS apt repository, without
# installing them.
#
#   ci/fetch-sdk-apt.sh <suite> <output dir>
#
# Needs root (it adds an apt source), curl and gnupg. The repository key is
# refused unless its fingerprint is the one Vitruvian's own CI pins, and each
# file is checked against the SHA256 in the signed index.
#
# The published amd64 packages sit under pool/pool/main/ while the index says
# pool/main/, so `apt-get download` gets a 404. Each file is fetched from the
# indexed path first and from the same path under pool/ as a fallback.
set -eu

SUITE="${1:?apt suite}"
OUT="${2:?output dir}"
URL="${VOS_REPO_URL:-https://repo.v-os.dev}"
FPR="${VOS_REPO_KEY_FPR:-8276B9B78A0853D17CB0CB0E9B50324D1D38BE53}"
KEY=/etc/apt/keyrings/vitruvian-archive-keyring.asc

install -d -m 755 /etc/apt/keyrings /etc/apt/sources.list.d
curl -fsSL -o "$KEY" "$URL/repo-key.asc"
got="$(gpg --show-keys --with-colons "$KEY" | awk -F: '/^fpr/{print $10; exit}')"
[ "$got" = "$FPR" ] || { echo "fetch-sdk: repo key fingerprint $got, expected $FPR" >&2; exit 1; }

cat > /etc/apt/sources.list.d/vitruvian.sources <<SRC
Types: deb
URIs: $URL
Suites: $SUITE
Components: main
Signed-By: $KEY
SRC
apt-get update

mkdir -p "$OUT"
for pkg in vos vos-dev vos-data; do
    # Only this repository's entry: the Debian archive has no such packages.
    info="$(apt-cache show --no-all-versions "$pkg" 2>/dev/null)" \
        || { echo "fetch-sdk: $pkg is not in suite $SUITE" >&2; exit 1; }
    file="$(printf '%s\n' "$info" | awk '/^Filename:/{print $2; exit}')"
    sha="$(printf '%s\n' "$info" | awk '/^SHA256:/{print $2; exit}')"
    dest="$OUT/$(basename "$file")"

    ok=0
    for base in "$URL/" "$URL/pool/"; do
        if curl -fsSL -o "$dest" "$base$file" 2>/dev/null; then
            echo "$sha  $dest" | sha256sum -c --quiet - && { ok=1; echo "fetch-sdk: $pkg from $base$file"; break; }
        fi
    done
    [ "$ok" = 1 ] || { echo "fetch-sdk: cannot download $pkg ($file)" >&2; exit 1; }
done
ls -la "$OUT"
