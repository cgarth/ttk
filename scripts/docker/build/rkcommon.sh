#! /bin/bash

set -e

# do not install if not on an Intel processor
[ $(uname -m) == "x86_64" ] || exit 0

# --------------------------------------------------------------------------

build_pkgs \
    build-essential         \
    curl                    \
    ca-certificates         \
    cmake                   \
    ninja-build             \
    libtbb-dev

runtime_pkgs \
    libtbb2

# --------------------------------------------------------------------------

# get source code
curl -L https://github.com/ospray/rkcommon/archive/v1.6.1.tar.gz | \
tar xz --strip-components 1

mkdir build
pushd build

cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release    \
    -DCMAKE_INSTALL_PREFIX=/usr   \
    -DBUILD_TESTING=OFF           \
    ..

cmake --build .
cmake --install .

popd