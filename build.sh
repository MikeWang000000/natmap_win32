#!/bin/bash
set -xeu
cd $(dirname $0)

if [ "${MSYSTEM:-}" != "MSYS" ]; then
    echo "Error: Please run this script on MSYS2" >&2
    exit 1
fi

pacman -Sy --needed --noconfirm gcc git make patch zip mingw-w64-clang-x86_64-libkqueue
export CFLAGS="-I/clang64/include -L/clang64/lib"

if [ ! -d natmap ]; then
    export MSYS=winsymlinks:native
    git clone --recursive "https://github.com/heiher/natmap"
    (cd natmap; patch -p1) < patches/natmap.win32.patch
    (cd natmap/third-part/hev-task-system; patch -p1) < patches/hev-task-system.win32.patch
fi

(cd natmap; make)

mkdir -p dist/natmap
cp natmap/bin/natmap /clang64/bin/libkqueue.dll /usr/bin/msys-2.0.dll dist/natmap

(cd dist; zip -r natmap_win32.zip natmap)
