# syntax=docker/dockerfile:1
#
# Builds vos-bepdf against the V\OS SDK without installing V\OS itself
# (see ci/setup-sdk.sh).
#
#   docker/build.sh <dir with VOS-*-Linux-{runtime,dev,data}.deb> [buildtools-dir]
#
# rc, xres, resattr and linkcatkeys shipped in vos are V\OS programs: they
# link libbe and need Nexus IPC, so they cannot run in a container. The host
# builds of the same tools (Vitruvian's buildtools/) are used instead.

FROM debian:trixie AS sdk
COPY ci/setup-sdk.sh /usr/local/sbin/setup-sdk.sh
RUN --mount=from=vos,target=/vos setup-sdk.sh /vos \
    && rm -rf /var/lib/apt/lists/*

FROM sdk AS build
COPY --from=tools rc xres resattr linkcatkeys /usr/local/bin/
COPY . /src
RUN cmake -S /src -B /build -G Ninja \
    && ninja -C /build \
    && ninja -C /build package

FROM scratch AS deb
COPY --from=build /build/vos-bepdf_*.deb /
