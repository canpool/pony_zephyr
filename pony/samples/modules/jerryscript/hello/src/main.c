/*
 * Copyright (c) 2025 maminjie <canpool@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <jerryscript.h>

int main(void)
{
	printk("Starting JerryScript demo...\n");

	jerry_init(JERRY_INIT_EMPTY);

	const char *js_code = "var a = 1 + 2 * 3; a";
	jerry_value_t result =
		jerry_eval((const jerry_char_t *)js_code, strlen(js_code), JERRY_PARSE_NO_OPTS);

	if (jerry_value_is_error(result)) {
		printk("JavaScript execution error\n");
	} else {
		jerry_value_t str_val = jerry_value_to_string(result);
		jerry_size_t str_size = jerry_string_size(str_val, JERRY_ENCODING_UTF8);
		char *str_buf = malloc(str_size + 1);

		jerry_string_to_buffer(str_val, JERRY_ENCODING_UTF8, (jerry_char_t *)str_buf,
				       str_size);
		str_buf[str_size] = '\0';

		printk("JavaScript result: %s\n", str_buf);

		free(str_buf);
		jerry_value_free(str_val);
	}

	jerry_value_free(result);
	jerry_cleanup();

	printk("JerryScript demo completed\n");

	return 0;
}
