/*
 * Copyright (c) 2025 maminjie <canpool@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include <lvgl_mem.h>
#include <lv_adv.h>
#include <stdio.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

int main(void)
{
	const struct device *display_dev;

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}
	lv_display_t *disp = lv_display_get_default();
	lv_theme_t *theme =
		lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE),
				      lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
	lv_disp_set_theme(disp, theme);

#ifdef CONFIG_BOARD_NATIVE_SIM
	lv_adv_sim_display_set_bg_color(lv_color_white());
#endif

	lv_adv_example_button_1();

	lv_timer_handler();
	display_blanking_off(display_dev);
#ifdef CONFIG_LV_Z_MEM_POOL_SYS_HEAP
	lvgl_print_heap_info(false);
#else
	printf("lvgl in malloc mode\n");
#endif
	while (1) {
#ifdef CONFIG_BOARD_NATIVE_SIM
		lv_adv_sim_screen_set_radius(LV_RADIUS_CIRCLE);
#endif
		uint32_t sleep_ms = lv_timer_handler();

		k_msleep(MIN(sleep_ms, INT32_MAX));
	}

	return 0;
}
