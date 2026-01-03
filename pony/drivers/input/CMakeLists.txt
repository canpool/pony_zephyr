# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_property(ALLOW_EMPTY TRUE)

# zephyr-keep-sorted-start
zephyr_library_sources_ifdef(CONFIG_INPUT_ST1633I input_st1633i.c)
# zephyr-keep-sorted-stop
