/**
 * Copyright (c) 2026 maminjie <canpool@163.com>
 * SPDX-License-Identifier: Apache-2.0
 */

/* Address by default is 0x70 */

#define DT_DRV_COMPAT sitronix_st1633i

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>
#include <stdbool.h>

LOG_MODULE_REGISTER(st1633i, CONFIG_INPUT_LOG_LEVEL);

/* Mask */
#define TOUCH_POINT_VALID_MSK      GENMASK(7, 7)

/* Offset for coordinates registers */
#define XY_COORD_H 0x2
#define X_COORD_L  0x3
#define Y_COORD_L  0x4

/* ST1633I configuration (DT) */
struct st1633i_config {
	/** I2C bus. */
	struct i2c_dt_spec bus;
#ifdef CONFIG_INPUT_ST1633I_INTERRUPT
	/** Interrupt GPIO information. */
	struct gpio_dt_spec int_gpio;
#endif
};

/* ST1633I data */
struct st1633i_data {
	/** Device pointer. */
	const struct device *dev;
	/** Work queue (for deferred read). */
	struct k_work work;
#ifdef CONFIG_INPUT_ST1633I_INTERRUPT
	/** Interrupt GPIO callback. */
	struct gpio_callback int_gpio_cb;
#else
	/* Timer (polling mode) */
	struct k_timer timer;
#endif
	/* Last pressed state */
	uint8_t pressed_old: 1;
	uint8_t pressed: 1;
};

static int st1633i_ts_init(const struct device *dev)
{
	return 0;
}

static int st1633i_process(const struct device *dev)
{
	const struct st1633i_config *config = dev->config;
	struct st1633i_data *data = dev->data;
	uint16_t y;
	uint16_t x;
	int ret;
	uint8_t buffer[5];

	ret = i2c_read_dt(&config->bus, buffer, sizeof(buffer));
	if (ret < 0) {
		LOG_ERR("Read coordinates failed: %d", ret);
		return ret;
	}

	/* Coordinates for one valid touch point */
	if (buffer[XY_COORD_H] & TOUCH_POINT_VALID_MSK) {
		x = (uint16_t)(buffer[XY_COORD_H] & 0x70) << 4 | buffer[X_COORD_L];
		y = (uint16_t)(buffer[XY_COORD_H] & 0x07) << 8 | buffer[Y_COORD_L];
		data->pressed = true;

		if (!data->pressed_old) {
			/* Finger pressed */
			input_report_abs(dev, INPUT_ABS_X, x, false, K_FOREVER);
			input_report_abs(dev, INPUT_ABS_Y, y, false, K_FOREVER);
			input_report_key(dev, INPUT_BTN_TOUCH, 1, true, K_FOREVER);
			LOG_DBG("Finger is touching x = %i y = %i", x, y);
		} else if (data->pressed_old) {
			/* Continuous pressed */
			input_report_abs(dev, INPUT_ABS_X, x, false, K_FOREVER);
			input_report_abs(dev, INPUT_ABS_Y, y, false, K_FOREVER);
			LOG_DBG("Finger keeps touching x = %i y = %i", x, y);
		}
	} else {
		data->pressed = false;

		if (data->pressed_old) {
			/* Finger removed */
			input_report_key(dev, INPUT_BTN_TOUCH, 0, true, K_FOREVER);
			LOG_DBG("Finger is removed");
		}
	}

	data->pressed_old = data->pressed;

	return 0;
}

static void st1633i_work_handler(struct k_work *work)
{
	struct st1633i_data *data = CONTAINER_OF(work, struct st1633i_data, work);

	st1633i_process(data->dev);
}

#ifdef CONFIG_INPUT_ST1633I_INTERRUPT

static void st1633i_isr_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	struct st1633i_data *data = CONTAINER_OF(cb, struct st1633i_data, int_gpio_cb);

	k_work_submit(&data->work);
}
#else
static void st1633i_timer_handler(struct k_timer *timer)
{
	struct st1633i_data *data = CONTAINER_OF(timer, struct st1633i_data, timer);

	k_work_submit(&data->work);
}
#endif

static int st1633i_init(const struct device *dev)
{
	const struct st1633i_config *config = dev->config;
	struct st1633i_data *data = dev->data;
	int ret;

	if (!i2c_is_ready_dt(&config->bus)) {
		LOG_ERR("I2C controller device not ready");
		return -ENODEV;
	}

	data->dev = dev;
	k_work_init(&data->work, st1633i_work_handler);

#ifdef CONFIG_INPUT_ST1633I_INTERRUPT

	LOG_DBG("Int conf for TS gpio: %p", &config->int_gpio);

	if (!gpio_is_ready_dt(&config->int_gpio)) {
		LOG_ERR("Interrupt GPIO controller device not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&config->int_gpio, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Could not configure interrupt GPIO pin");
		return ret;
	}

	gpio_init_callback(&data->int_gpio_cb, st1633i_isr_handler, BIT(config->int_gpio.pin));

	ret = gpio_add_callback(config->int_gpio.port, &data->int_gpio_cb);
	if (ret < 0) {
		LOG_ERR("Could not set gpio callback");
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&config->int_gpio, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Could not configure interrupt GPIO interrupt.");
		return ret;
	}
#else
	LOG_DBG("Timer Mode");
	k_timer_init(&data->timer, st1633i_timer_handler, NULL);
	k_timer_start(&data->timer, K_MSEC(CONFIG_INPUT_ST1633I_PERIOD_MS),
		      K_MSEC(CONFIG_INPUT_ST1633I_PERIOD_MS));
#endif

	ret = st1633i_ts_init(dev);
	if (ret < 0) {
		LOG_ERR("Init information of sensor failed: %d", ret);
		return ret;
	}
	return 0;
}

#define ST1633I_DEFINE(index)																\
	static const struct st1633i_config st1633i_config_##index = {							\
		.bus = I2C_DT_SPEC_INST_GET(index),													\
		IF_ENABLED(CONFIG_INPUT_ST1633I_INTERRUPT,											\
			(.int_gpio = GPIO_DT_SPEC_INST_GET(index, int_gpios),))							\
	};																						\
	static struct st1633i_data st1633i_data_##index;										\
																							\
	DEVICE_DT_INST_DEFINE(index, st1633i_init, NULL, &st1633i_data_##index,					\
		&st1633i_config_##index, POST_KERNEL, CONFIG_INPUT_INIT_PRIORITY,					\
		NULL);

DT_INST_FOREACH_STATUS_OKAY(ST1633I_DEFINE);
