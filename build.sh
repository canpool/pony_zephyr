#!/usr/bin/env bash
# Copyright (c) 2025 maminjie <canpool@163.com>
# SPDX-License-Identifier: Apache-2.0

readonly TOP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
bash $TOP_DIR/scripts/build.sh "$@"
