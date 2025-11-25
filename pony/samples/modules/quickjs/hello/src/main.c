/*
 * Copyright (c) 2025 maminjie <canpool@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <quickjs.h>

int main(void)
{
	JSRuntime *rt;
	JSContext *ctx;
	JSValue result;
	const char *script = "1 + 2 * 3";

	printk("Starting QuickJS demo...\n");

	rt = JS_NewRuntime();
	ctx = JS_NewContext(rt);

	result = JS_Eval(ctx, script, strlen(script), "<inline>", JS_EVAL_TYPE_GLOBAL);

	if (JS_IsException(result)) {
		printk("JavaScript execution error\n");
	} else {
		const char *str = JS_ToCString(ctx, result);
		printk("Result: %s\n", str);
		JS_FreeCString(ctx, str);
		JS_FreeValue(ctx, result);
	}

	JS_FreeContext(ctx);
	JS_FreeRuntime(rt);

	printk("QuickJS demo completed\n");

	return 0;
}
