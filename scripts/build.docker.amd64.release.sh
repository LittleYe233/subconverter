#!/bin/sh
set -xe

# A simple Docker build script from GitHub CI, for Linux amd64

WORKDIR=$(dirname "$0")/..
cd "$WORKDIR"

# Default to keep commit SHA in version
SHA=$(git rev-parse --short HEAD) && sed -i 's/\(v[0-9]\.[0-9]\.[0-9]\)/\1-'"$SHA"'/' src/version.h

docker run --rm -v "$WORKDIR":/root/workdir multiarch/alpine:amd64-latest-stable /bin/sh -c "apk add bash git nodejs npm && cd /root/workdir && chmod +x scripts/build.alpine.release.sh && bash scripts/build.alpine.release.sh"
