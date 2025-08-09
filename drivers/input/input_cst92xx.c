/*
 * Copyright (c) 2021 Qingsong Gou <gouqs@hotmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT hynitron_cst92xx

#include <stdint.h>

#include <zephyr/sys/byteorder.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>
#include <zephyr/dt-bindings/input/cst92xx-gesture-codes.h>
#include <zephyr/drivers/input/input_touchscreen.h>

#include "input_cst92xx.h"

LOG_MODULE_REGISTER(cst92xx, CONFIG_INPUT_LOG_LEVEL);

#define CST92XX_TOUCH_COORD_BUF_SIZE (CST92XX_MAX_TOUCH_POINTS * 5 + 5)

int touchscreen_dev_config(struct device *dev, const enum touchscreen_dev_cfg_mask param_mask, const struct input_touchscreen_common_config * cfg) {
	struct cst92xx_data *data = dev->data;
	int ret = 0;

	if ((param_mask &
		(~(TOUCHSCREEN_DEVICE_CONFIG_SCREEN_WIDTH | TOUCHSCREEN_DEVICE_CONFIG_SCREEN_HEIGHT |
	   TOUCHSCREEN_DEVICE_CONFIG_INVERTED_X | TOUCHSCREEN_DEVICE_CONFIG_INVERTED_Y |
	   TOUCHSCREEN_DEVICE_CONFIG_SWAPPED_X_Y | TOUCHSCREEN_DEVICE_CONFIG_ALL)))) {
	   LOG_ERR("Configuration type not supported.");
	   return -ENOTSUP;
	}

	if (param_mask & TOUCHSCREEN_DEVICE_CONFIG_SCREEN_WIDTH) {
		data->touchscreen.screen_width = cfg->screen_width;
	}

	if (param_mask & TOUCHSCREEN_DEVICE_CONFIG_SCREEN_HEIGHT) {
		data->touchscreen.screen_height = cfg->screen_height;
	}

	if (param_mask & TOUCHSCREEN_DEVICE_CONFIG_INVERTED_X) {
		data->touchscreen.inverted_x = cfg->inverted_x;
	}

	if (param_mask & TOUCHSCREEN_DEVICE_CONFIG_INVERTED_Y) {
		data->touchscreen.inverted_y = cfg->inverted_y;
	}

	if (param_mask & TOUCHSCREEN_DEVICE_CONFIG_SWAPPED_X_Y) {
		data->touchscreen.swapped_x_y = cfg->swapped_x_y;
	}

	if (param_mask & TOUCHSCREEN_DEVICE_CONFIG_ALL) {
		data->touchscreen.screen_width = cfg->screen_width;
		data->touchscreen.screen_height = cfg->screen_height;
		data->touchscreen.inverted_x = cfg->inverted_x;
		data->touchscreen.inverted_y = cfg->inverted_y;
		data->touchscreen.swapped_x_y = cfg->swapped_x_y;
	}

	return ret;
}

static int cst92xx_write_command(const struct i2c_dt_spec *i2c, uint16_t command)
{
	int ret = 0;
	uint16_t cmd = sys_cpu_to_be16(command);

	ret = i2c_write_dt(i2c, (uint8_t *)&cmd, 2);
	if (ret < 0) {
		LOG_ERR("Failed to write command 0x%x", command);
		return ret;
	}

	return ret;
}

static int cst92xx_write_ack(const struct i2c_dt_spec *i2c, uint16_t command)
{
	int ret = 0;
	uint8_t ack_buffer[3] = { (uint8_t)((command >> 8) & 0xFF), (uint8_t)(command & 0xFF), CST92XX_W_ACK };

	ret = i2c_write_dt(i2c, ack_buffer, 3);
	if (ret < 0) {
		LOG_ERR("Failed to write ack for command 0x%x", command);
		return ret;
	}

	return ret;
}

static int cst92xx_read_data(const struct i2c_dt_spec *i2c, uint16_t command, uint8_t *read_buffer, size_t num_read)
{
	int ret = 0;
	uint16_t cmd = sys_cpu_to_be16(command);

	ret = i2c_write_read_dt(i2c, &cmd, 2, read_buffer, num_read);
	if (ret < 0) {
		LOG_ERR("Failed to read data for command 0x%x", command);
		return ret;
	}

	return ret;
}

static struct cst92xx_touch_point cst92xx_parse_touch_point(const struct device *dev, uint8_t *points_data, int point_index)
{
	const struct cst92xx_config *config = dev->config;
	const struct cst92xx_data *data = dev->data;

	uint8_t *point_data = &points_data[point_index * 5 + (point_index ? 2 : 0)];

	int16_t x = ((point_data[1] << 4) | (point_data[3] >> 4));
	int16_t y = ((point_data[2] << 4) | (point_data[3] & 0x0F));

	if (data->touchscreen.inverted_x) {
		x = data->touchscreen.screen_width - x;
	}

	if (data->touchscreen.inverted_y) {
		y = data->touchscreen.screen_height - y;
	}

	if (data->touchscreen.swapped_x_y) {
		int16_t temp = x;
		x = y;
		y = temp;
	}

	struct cst92xx_touch_point touch_point = {
		.index = point_index,
		.status = point_data[0] & 0x0F,
		.x = x,
		.y = y,
	};

	return touch_point;
}

static int cst92xx_process(const struct device *dev)
{
	const struct cst92xx_config *config = dev->config;
	const struct cst92xx_data *data = dev->data;
	int ret = 0;

	uint8_t read_buffer[CST92XX_TOUCH_COORD_BUF_SIZE] = {0};

	ret = cst92xx_read_data(&config->i2c, CST92XX_R_COORDINATES, read_buffer, sizeof(read_buffer));
	if (ret < 0) {
		LOG_ERR("Failed to read coordinates");
		return ret;
	}

	ret = cst92xx_write_ack(&config->i2c, CST92XX_R_COORDINATES);
	if (ret < 0) {
		return ret;
	}

	if (read_buffer[6] != CST92XX_W_ACK) {
		LOG_ERR("Check device ack error , response code is  0x%x", read_buffer[6]);
		return -EINVAL;
	}

	uint8_t touch_count = (read_buffer[5] & CST92XX_TOUCH_IND_MASK);
    if (touch_count > CST92XX_MAX_TOUCH_POINTS || touch_count == 0) {
		LOG_ERR("Invalid number of touch points %d", touch_count);
        return -EINVAL;
    }

	for (int i = 0; i < touch_count; i++) {
        struct cst92xx_touch_point touch_point = cst92xx_parse_touch_point(dev, read_buffer, i);

		if (touch_count > 1) {
			input_report_abs(dev, INPUT_ABS_MT_SLOT, i, true, K_FOREVER);
		}

        if (touch_point.status == CST92XX_STATUS_PRESSED) {
			input_report_abs(dev, INPUT_ABS_X, touch_point.x, false, K_FOREVER);
			input_report_abs(dev, INPUT_ABS_Y, touch_point.y, false, K_FOREVER);
			input_report_key(dev, INPUT_BTN_TOUCH, 1, true, K_FOREVER);
		} else {
			input_report_key(dev, INPUT_BTN_TOUCH, 0, true, K_FOREVER);
		}
    }

#ifdef CONFIG_INPUT_CST92XX_EV_DEVICE
	uint8_t gesture = CST92XX_GESTURE_CODE_NONE;

	if (read_buffer[4] & 0xF0) {
		gesture = read_buffer[4] >> 4;
	}

	if (gesture != CST92XX_GESTURE_CODE_NONE) {
		input_report(dev, INPUT_EV_DEVICE, (uint16_t)gesture, 0, true, K_FOREVER);
		LOG_DBG("Gesture detected: %d", gesture);
	}
#endif

	return 0;
}

static void cst92xx_work_handler(struct k_work *work)
{
	struct cst92xx_data *data = CONTAINER_OF(work, struct cst92xx_data, work);

	cst92xx_process(data->dev);

	atomic_set(&data->work_pending, 0);
}

#ifdef CONFIG_INPUT_CST92XX_INTERRUPT
static void cst92xx_isr_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	struct cst92xx_data *data = CONTAINER_OF(cb, struct cst92xx_data, int_gpio_cb);

	if (atomic_cas(&data->work_pending, 0, 1)) {
        k_work_submit(&data->work);
    }
}
#else
static void cst92xx_timer_handler(struct k_timer *timer)
{
	struct cst92xx_data *data = CONTAINER_OF(timer, struct cst92xx_data, timer);

	k_work_submit(&data->work);
}
#endif

static int cst92xx_chip_reset(const struct device *dev)
{
	const struct cst92xx_config *config = dev->config;
	int ret = 0;

	if (gpio_is_ready_dt(&config->rst_gpio)) {
		ret = gpio_pin_configure_dt(&config->rst_gpio, GPIO_OUTPUT_ACTIVE);
		if (ret < 0) {
			LOG_ERR("Could not configure reset GPIO pin");
			return ret;
		}

		k_msleep(CST92XX_RESET_DELAY_MS);

		gpio_pin_set_dt(&config->rst_gpio, 0);
		k_msleep(CST92XX_WAIT_DELAY_MS);
	}

	return ret;
}

static int cst92xx_chip_init(const struct device *dev)
{
	const struct cst92xx_config *config = dev->config;
	struct cst92xx_data *data = dev->data;

	int ret = 0;

	LOG_INF("I2C Address: %d", config->i2c.addr);

	ret = cst92xx_chip_reset(dev);
	if (ret < 0) {
		return ret;
	}

	if (!device_is_ready(config->i2c.bus)) {
		LOG_ERR("I2C bus %s not ready", config->i2c.bus->name);
		return -ENODEV;
	}

	uint8_t read_buffer[8];

	ret = cst92xx_write_command(&config->i2c, CST92XX_C_CMD_MODE);
	if (ret < 0) {
		LOG_ERR("Failed enter command mode");
		return ret;
	}
	k_msleep(CST92XX_CMD_MODE_DELAY_MS);

	ret = cst92xx_read_data(&config->i2c, CST92XX_R_CHECKCODE, read_buffer, 4);
	if (ret < 0) {
		LOG_ERR("Failed to read chip check code");
		return ret;
	}

    uint32_t checkcode = read_buffer[3];
    checkcode <<= 8;
    checkcode |= read_buffer[2];
    checkcode <<= 8;
    checkcode |= read_buffer[1];
    checkcode <<= 8;
    checkcode |= read_buffer[0];

	if ((checkcode & 0xFFFF0000) != CST92XX_FW_VER_CHECKCODE) {
		LOG_ERR("Firmware info read error.");
		return -ENODEV;
	}

	LOG_DBG("Chip check code: 0x%x", checkcode);

	ret = cst92xx_read_data(&config->i2c, CST92XX_R_RESOLUTION, read_buffer, 4);
	if (ret < 0) {
		LOG_ERR("Failed to read chip resolution");
		return ret;
	}

	data->touchscreen.screen_width = (read_buffer[1] << 8) | read_buffer[0];
    data->touchscreen.screen_height = (read_buffer[3] << 8) | read_buffer[2];

	LOG_DBG("Chip resolution X: %d, Y: %d", data->touchscreen.screen_width, data->touchscreen.screen_height);

	ret = cst92xx_read_data(&config->i2c, CST92XX_R_CHIP_TYPE, read_buffer, 4);
	if (ret < 0) {
		LOG_ERR("Failed to read chip type");
		return ret;
	}

	uint16_t chip_type = (read_buffer[3] << 8) | read_buffer[2];

	uint32_t project_id = read_buffer[1];
    project_id <<= 8;
    project_id |= read_buffer[0];

	if ((chip_type != CST9220_CHIP_ID) && (chip_type != CST9217_CHIP_ID)) {
		LOG_ERR("Chip type error 0x%x", chip_type);
		return -ENODEV;
	}

	LOG_DBG("Chip type: 0x%x, Project ID: 0x%x", chip_type, project_id);

	ret = cst92xx_read_data(&config->i2c, CST92XX_R_FIRMWARE_VERSION, read_buffer, 8);
	if (ret < 0) {
		LOG_ERR("Failed to read chip firmware version");
		return ret;
	}

	uint32_t fw_version = read_buffer[3];
    fw_version <<= 8;
    fw_version |= read_buffer[2];
    fw_version <<= 8;
    fw_version |= read_buffer[1];
    fw_version <<= 8;
    fw_version |= read_buffer[0];

	uint32_t checksum = read_buffer[7];
    checksum <<= 8;
    checksum |= read_buffer[6];
    checksum <<= 8;
    checksum |= read_buffer[5];
    checksum <<= 8;
    checksum |= read_buffer[4];

	if (fw_version == CST92XX_FW_VER_BLANK) {
		LOG_ERR("Chip ic don't have firmware.");
		return -ENODEV;
	}

	LOG_DBG("Chip firmware version: 0x%x, checksum: 0x%x", fw_version, checksum);

	return ret;
}

static int cst92xx_init(const struct device *dev)
{
	LOG_INF("Initializing CST92XX driver");

	struct cst92xx_data *data = dev->data;
	int ret;

	data->dev = dev;
	k_work_init(&data->work, cst92xx_work_handler);

	ret = cst92xx_chip_init(dev);
	if (ret < 0) {
		return ret;
	}

#ifdef CONFIG_INPUT_CST92XX_INTERRUPT
	const struct cst92xx_config *config = dev->config;

	if (!gpio_is_ready_dt(&config->int_gpio)) {
		LOG_ERR("GPIO port %s not ready", config->int_gpio.port->name);
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&config->int_gpio, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Could not configure interrupt GPIO pin");
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&config->int_gpio, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Could not configure interrupt GPIO interrupt.");
		return ret;
	}

	gpio_init_callback(&data->int_gpio_cb, cst92xx_isr_handler, BIT(config->int_gpio.pin));

	ret = gpio_add_callback(config->int_gpio.port, &data->int_gpio_cb);
	if (ret < 0) {
		LOG_ERR("Could not set gpio callback");
		return ret;
	}
#else
	k_timer_init(&data->timer, cst92xx_timer_handler, NULL);
	k_timer_start(&data->timer, K_MSEC(CONFIG_INPUT_CST92XX_PERIOD),
		      K_MSEC(CONFIG_INPUT_CST92XX_PERIOD));
#endif

	return ret;
}

#define CST92XX_DEFINE(index)                                            	\
	static const struct cst92xx_config cst92xx_config_##index = {           \
		.i2c = I2C_DT_SPEC_INST_GET(index),                                 \
		COND_CODE_1(CONFIG_INPUT_CST92XX_INTERRUPT,                         \
			(.int_gpio = GPIO_DT_SPEC_INST_GET(index, irq_gpios),), ())     \
		.rst_gpio = GPIO_DT_SPEC_INST_GET_OR(index, rst_gpios, {}),         \
	};                                                                      \
	static struct cst92xx_data cst92xx_data_##index = {						\
		.touchscreen = INPUT_TOUCH_DT_INST_COMMON_CONFIG_INIT(index),		\
	};																		\
	DEVICE_DT_INST_DEFINE(index, cst92xx_init, NULL, &cst92xx_data_##index, \
		&cst92xx_config_##index, POST_KERNEL, CONFIG_INPUT_INIT_PRIORITY,   \
		NULL);

DT_INST_FOREACH_STATUS_OKAY(CST92XX_DEFINE)
