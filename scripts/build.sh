#!/usr/bin/env bash
# Copyright (c) 2025 maminjie <canpool@163.com>
# SPDX-License-Identifier: Apache-2.0

CUR_DIR=$(pwd)
OUT_DIR=$CUR_DIR/out
BUILD_DIR=""
BUILD_ARGS=()

usage() {
printf "wrapper of west build

usage:
    bash build.sh [-h] [-q OUT_DIR] [...]

optional arguments:
    -q OUT_DIR     output directory to create or use
    -h             show the help message and exit

other argmuments:
    [...]          west build -h for more details
\n"
}

output() {
    if [ -z "$BUILD_DIR" ]; then
        return
    fi
    local build_dir=$(realpath "$BUILD_DIR")
    mkdir -p $OUT_DIR
    local out_dir=$(realpath "$OUT_DIR")
    cp $build_dir/zephyr/zephyr.{dts,elf,map} $out_dir
    if [ -f "$build_dir/zephyr/zephyr.hex" ]; then
        cp $build_dir/zephyr/zephyr.hex $out_dir
    else
        rm -f $out_dir/zephyr.hex
    fi
    if [ -f "$build_dir/zephyr/zephyr.bin" ]; then
        cp $build_dir/zephyr/zephyr.bin $out_dir
    else
        rm -f $out_dir/zephyr.bin
    fi
    cp $build_dir/zephyr/.config $out_dir
    cp $build_dir/Kconfig/Kconfig.dts $out_dir
    cp $build_dir/zephyr/include/generated/zephyr/autoconf.h $out_dir
    cp $build_dir/zephyr/include/generated/zephyr/devicetree_generated.h $out_dir

    echo "Success, outputing files to $out_dir:"
    if [ -f "$out_dir/zephyr.hex" ]; then
        size $out_dir/zephyr.{elf,hex}
    else
        size $out_dir/zephyr.elf
    fi
}

# filter_args option has_value "$@"
filter_args() {
    local option="$1"       # such as: "-q"
    local has_value="$2"    # such as: "y" or "n"
    shift 2

    BUILD_ARGS=()
    local arg skip_next=false

    for arg in "$@"; do
        if $skip_next; then
            skip_next=false
            continue
        fi

        if [[ "$arg" == "$option" ]]; then
            [[ "$has_value" == "y" ]] && skip_next=true
            continue
        fi

        BUILD_ARGS+=("$arg")
    done
}

main() {
    if [ $# -eq 0 ]; then
        usage; exit 0;
    fi

    while getopts ":d:q:h" opt; do
        case "$opt" in
            h)
                usage; exit 0
            ;;
            d)
                BUILD_DIR="$OPTARG"
            ;;
            q)
                OUT_DIR="$OPTARG"
            ;;
            ?)
                break
            ;;
        esac
    done

    filter_args "-q" "y" "$@"

    west build "${BUILD_ARGS[@]}"
    ret=$?
    if [ $ret -eq 0 ]; then
        output
    fi

    return $ret
}

main "$@"
