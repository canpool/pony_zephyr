#!/usr/bin/env bash
# Copyright (c) 2025 maminjie <canpool@163.com>
# SPDX-License-Identifier: Apache-2.0

CUR_DIR=$(pwd)
OUT_DIR=$CUR_DIR/out
BUILD_DIR=""

usage() {
    west build -h | sed 's|usage: west build|usage: bash build.sh|g'
}

output() {
    if [ -z "$BUILD_DIR" ]; then
        return
    fi
    local build_dir=$(realpath "$BUILD_DIR")
    mkdir -p $OUT_DIR
    cp $build_dir/zephyr/zephyr.{bin,dts,elf,hex,map} $OUT_DIR
    cp $build_dir/zephyr/.config $OUT_DIR
    cp $build_dir/Kconfig/Kconfig.dts $OUT_DIR
    cp $build_dir/zephyr/include/generated/zephyr/autoconf.h $OUT_DIR
    cp $build_dir/zephyr/include/generated/zephyr/devicetree_generated.h $OUT_DIR

    echo "Success, outputing files to $OUT_DIR:"
    size $OUT_DIR/zephyr.{elf,hex}
}

main() {
    while getopts ":d:h" opt; do
        case "$opt" in
            h)
                usage; exit 0
            ;;
            d)
                BUILD_DIR="$OPTARG"
            ;;
            ?)
                break
            ;;
        esac
    done

    west build "$@"
    ret=$?
    if [ $ret -eq 0 ]; then
        output
    fi

    return $ret
}

main "$@"
