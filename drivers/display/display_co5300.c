#define DT_DRV_COMPAT chipone_co5300

#include "display_co5300.h"

#include <zephyr/dt-bindings/display/panel.h>
#include <zephyr/drivers/mspi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/devicetree.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(display_co5300, CONFIG_DISPLAY_LOG_LEVEL);

struct co5300_config {
	const struct device* mspi_bus;
	struct mspi_dev_id dev_id;
	struct mspi_dev_cfg mspi_dev_cfg;
	const struct gpio_dt_spec dc_gpios;
	const struct gpio_dt_spec reset_gpios;
	uint16_t height;
	uint16_t width;
	uint8_t pixel_format;
	bool write_only;
	uint8_t orientation;
	bool display_invert;
	bool color_invert;
	uint32_t sram_size;
	int max_buf_size;
};

/* Display data struct */
struct co5300_data {
	struct k_mutex lock;
	uint8_t bytes_per_pixel;
	enum display_pixel_format pixel_format;
	enum display_orientation orientation;
	struct mspi_dev_cfg dev_cfg;
	bool configured;
};

static int co5300_transmit(const struct device *dev, uint8_t cmd, const void *tx_data, size_t tx_len, bool quad_mode) {
	const struct co5300_config* config = dev->config;
	struct co5300_data *data = dev->data;
	int ret = 0;

	ret = k_mutex_lock(&data->lock, K_FOREVER);
	if (ret < 0)
		return ret;

	struct mspi_dev_cfg mspi_dev_cfg;
	mspi_dev_cfg.io_mode = quad_mode ? MSPI_IO_MODE_QUAD_1_1_4 : MSPI_IO_MODE_SINGLE;
	mspi_dev_config(config->mspi_bus, &config->dev_id,
		MSPI_DEVICE_CONFIG_IO_MODE, &mspi_dev_cfg);

	struct mspi_xfer_packet cmd_packet = {
		.dir = MSPI_TX,
		.cmd = 0x02,
		.address = (uint32_t)cmd << 8,
		.data_buf = (void *)tx_data,
		.num_bytes = tx_len,
	};

	struct mspi_xfer cmd_xfer = {
		.packets = &cmd_packet,
		.num_packet = 1,
		.async = false,
		.timeout = 100,
		.priority = 0,
		.hold_ce = false,
		.cmd_length = 1,   // 8-bit command
		.addr_length = 3, // 24-bit address
	};

	/* Send command with data */
	ret = mspi_transceive(config->mspi_bus, &config->dev_id, &cmd_xfer);
	if (ret < 0) {
		LOG_ERR("Failed to send command 0x%02x: %d", cmd, ret);
	}

	k_mutex_unlock(&data->lock);
	return ret;
}

static int co5300_reset(const struct device *dev) {
	const struct co5300_config* config = dev->config;

	gpio_pin_set_dt(&config->reset_gpios, 0);
	k_sleep(K_MSEC(300));
	gpio_pin_set_dt(&config->reset_gpios, 1);
	k_sleep(K_MSEC(200));

	return 0;
}

static int co5300_blanking_off(const struct device *dev) {
	LOG_DBG("Turning display blanking off");

	int ret = co5300_transmit(dev, CO5300_C_SLPOUT, NULL, 0, false);
	if (ret < 0)
		return ret;
	k_sleep(K_MSEC(CO5300_SLPOUT_DELAY));

	ret = co5300_transmit(dev, CO5300_C_DISPON, NULL, 0, false);
	if (ret < 0)
		return ret;
	k_sleep(K_MSEC(10));

	return 0;
}

static int co5300_blanking_on(const struct device *dev) {
	LOG_DBG("Turning display blanking on");

	int ret = co5300_transmit(dev, CO5300_C_DISPOFF, NULL, 0, false);
	if (ret < 0)
		return ret;
	k_sleep(K_MSEC(CO5300_SLPIN_DELAY));

	ret = co5300_transmit(dev, CO5300_C_SLPIN, NULL, 0, false);
	if (ret < 0)
		return ret;
	k_sleep(K_MSEC(CO5300_SLPIN_DELAY));

	return 0;
}

static int co5300_set_pixel_format(const struct device *dev, const enum display_pixel_format pixel_format) {
	struct co5300_data *data = dev->data;
	uint8_t tx_data;
	uint8_t bytes_per_pixel;

	if (pixel_format == PIXEL_FORMAT_RGB_565) {
		bytes_per_pixel = 2U;
		tx_data = CO5300_PIXFMT_16BIT_RGB565;
	} else if (pixel_format == PIXEL_FORMAT_RGB_888) {
		bytes_per_pixel = 3U;
		tx_data = CO5300_PIXFMT_24BIT_RGB888;
	} else {
		LOG_ERR("Unsupported pixel format");
		return -ENOTSUP;
	}

	int ret = co5300_transmit(dev, CO5300_W_PIXFMT, &tx_data, 1U, false);
	if (ret < 0) {
		return ret;
	}

	data->pixel_format = pixel_format;
	data->bytes_per_pixel = bytes_per_pixel;

	return 0;
}

static int co5300_set_orientation(const struct device *dev, const enum display_orientation orientation) {
	struct co5300_data *data = dev->data;
	uint8_t tx_data = CO5300_MADCTL_RGB;

	switch (orientation) {
	case DISPLAY_ORIENTATION_NORMAL:
		/* No additional flags needed */
		break;
	case DISPLAY_ORIENTATION_ROTATED_90:
		tx_data |= CO5300_MADCTL_X_AXIS_FLIP;
		break;
	case DISPLAY_ORIENTATION_ROTATED_180:
		tx_data |= CO5300_MADCTL_X_AXIS_FLIP | CO5300_MADCTL_Y_AXIS_FLIP;
		break;
	case DISPLAY_ORIENTATION_ROTATED_270:
		tx_data |= CO5300_MADCTL_Y_AXIS_FLIP;
		break;
	default:
		LOG_ERR("Unsupported orientation: %d", orientation);
		return -ENOTSUP;
	}

	int ret = co5300_transmit(dev, CO5300_W_MADCTL, &tx_data, 1U, false);
	if (ret < 0) {
		return ret;
	}

	data->orientation = orientation;

	return 0;
}

static int co5300_set_window(const struct device *dev, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end) {
	uint8_t tx_data[4];
	int ret;

	tx_data[0] = x_start >> 8;
	tx_data[1] = x_start;
	tx_data[2] = x_end >> 8;
	tx_data[3] = x_end;

	ret = co5300_transmit(dev, CO5300_W_CASET, tx_data, 4, false);
	if (ret < 0) {
		return ret;
	}

	/* Set page address */
	tx_data[0] = y_start >> 8;
	tx_data[1] = y_start;
	tx_data[2] = y_end >> 8;
	tx_data[3] = y_end;

	ret = co5300_transmit(dev, CO5300_W_PASET, tx_data, 4, false);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int co5300_write(const struct device *dev, const uint16_t x, const uint16_t y,
	const struct display_buffer_descriptor *desc, const void *buf) {

	const struct co5300_config* config = dev->config;
	struct co5300_data *data = dev->data;
	int ret;

	__ASSERT(desc->width != 0U, "Width is not allowed to be zero");
	__ASSERT(desc->height != 0U, "Height is not allowed to be zero");
	__ASSERT(data->bytes_per_pixel * desc->pitch <= desc->buf_size, "Input buffer too small");

	LOG_DBG("Writing %dx%d (w,h) @ %dx%d (x,y)", desc->width, desc->height, x, y);
	LOG_DBG("Bytes per pixel: %d", data->bytes_per_pixel);

	ret = k_mutex_lock(&data->lock, K_FOREVER);
	if (ret < 0) {
		return ret;
	}

	/* Set drawing window */
	ret = co5300_set_window(dev, x, y, x + desc->width - 1, y + desc->height - 1);
	if (ret < 0) {
		goto unlock;
	}

	ret = co5300_transmit(dev, CO5300_W_RAMWR, NULL, 0, false);
	if (ret < 0) {
		goto unlock;
	}

	const uint8_t *src = (const uint8_t *)buf;
	int total_len = desc->height * desc->width * data->bytes_per_pixel;
	LOG_INF("Write info: %d %d %d", desc->height, desc->width, data->bytes_per_pixel);
	int remaining = total_len;
	bool first_send = true;
	bool is_last_send = false;

	struct mspi_dev_cfg mspi_dev_cfg;
	mspi_dev_cfg.io_mode = MSPI_IO_MODE_QUAD_1_1_4;
	mspi_dev_config(config->mspi_bus, &config->dev_id,
		MSPI_DEVICE_CONFIG_IO_MODE, &mspi_dev_cfg);

	while (remaining > 0) {
		LOG_DBG("Remaining %d bytes", remaining);

		int chunk_size = 0;
		if (remaining <= config->max_buf_size * 2) {
			is_last_send = true;
			chunk_size = remaining;
		} else {
			chunk_size = config->max_buf_size * 2;
		}

		struct mspi_xfer_packet data_packet = {
			.dir = MSPI_TX,
			.data_buf = (void *)src,
			.num_bytes = chunk_size,
		};

		struct mspi_xfer data_xfer = {
			.packets = &data_packet,
			.num_packet = 1,
			.async = false,
			.timeout = 100,
			.priority = 0,
			.hold_ce = !is_last_send,
		};

		if (first_send) {
			data_packet.cmd = 0x32;  // QIO write command
			data_packet.address = 0x002C00;  // Memory write in address field
			data_xfer.cmd_length = 1;
			data_xfer.addr_length = 3;
			first_send = false;
		} else {
			/* Subsequent transfers are data-only */
			data_packet.cmd = 0;
			data_packet.address = 0;
			data_xfer.cmd_length = 0;
			data_xfer.addr_length = 0;
		}

		ret = mspi_transceive(config->mspi_bus, &config->dev_id, &data_xfer);
		if (ret < 0) {
			LOG_ERR("Failed to send pixel data chunk: %d", ret);
			goto unlock;
		}

		src += chunk_size;
		remaining -= chunk_size;
	}

unlock:
	k_mutex_unlock(&data->lock);
	return ret;
}

static void co5300_get_capabilities(const struct device *dev, struct display_capabilities *capabilities) {
	struct co5300_data *data = dev->data;
	const struct co5300_config *config = dev->config;

	memset(capabilities, 0, sizeof(struct display_capabilities));

	capabilities->supported_pixel_formats = PIXEL_FORMAT_RGB_565 | PIXEL_FORMAT_RGB_888;
	capabilities->current_pixel_format = data->pixel_format;

	if (data->orientation == DISPLAY_ORIENTATION_NORMAL ||
		data->orientation == DISPLAY_ORIENTATION_ROTATED_180) {
		capabilities->x_resolution = config->width;
		capabilities->y_resolution = config->height;
	} else {
		capabilities->x_resolution = config->height;
		capabilities->y_resolution = config->width;
	}

	capabilities->current_orientation = data->orientation;
}

static int co5300_configure_display(const struct device *dev) {
	const struct co5300_config* config = dev->config;
	int ret;

// Try 10 times for now to make sure it works
for(int i = 0; i < 1; i++) {
	// Exit sleep mode
	ret = co5300_transmit(dev, CO5300_C_SLPOUT, NULL, 0, false);
	if (ret < 0)
		return ret;
	k_sleep(K_MSEC(CO5300_SLPOUT_DELAY));

	// Set SPI mode
	uint8_t tx_data = 0x80;
	ret = co5300_transmit(dev, CO5300_W_SPIMODECTL, &tx_data, 1, false);
	if (ret < 0)
		return ret;

	// Set pixel format
	ret = co5300_set_pixel_format(dev, PIXEL_FORMAT_RGB_565);
	if (ret < 0)
		return ret;

	// Write CTRL Display1
	tx_data = 0x20;
	ret = co5300_transmit(dev, CO5300_W_WCTRLD1, &tx_data, 1, false);
	if (ret < 0)
		return ret;

	// Write Display Brightness Value in HBM Mode
	tx_data = 0xFF;
	ret = co5300_transmit(dev, CO5300_W_WDBRIGHTNESSVALHBM, &tx_data, 1, false);
	if (ret < 0)
		return ret;

	// Display on
	ret = co5300_transmit(dev, CO5300_C_DISPON, NULL, 0, false);
	if (ret < 0)
		return ret;
	k_sleep(K_MSEC(10));

	// Write Display Brightness Value in Normal Mode
	tx_data = 0xFF;
	ret = co5300_transmit(dev, CO5300_W_WDBRIGHTNESSVALNOR, &tx_data, 1, false);
	if (ret < 0)
		return ret;
}

	LOG_DBG("CO5300 display configured successfully");

	return 0;
}

static int co5300_init(const struct device *dev) {
	const struct co5300_config* config = dev->config;
	struct co5300_data *data = dev->data;
	int ret;

	if (!device_is_ready(config->mspi_bus)) {
		LOG_ERR("MSPI device is not ready");
		return -ENODEV;
	}

	ret = mspi_dev_config(config->mspi_bus, &config->dev_id,
		MSPI_DEVICE_CONFIG_IO_MODE, &config->mspi_dev_cfg);
	if (ret < 0) {
		LOG_ERR("Failed to configure device: %d", ret);
		return ret;
	}

	/* Initialize mutex */
	k_mutex_init(&data->lock);

	/* Configure GPIOs - no DC pin needed for QSPI */
	if (config->dc_gpios.port) {
		ret = gpio_pin_configure_dt(&config->dc_gpios, GPIO_OUTPUT);
		if (ret < 0) {
			LOG_ERR("Could not configure command/data GPIO (%d)", ret);
			return ret;
		}
	}

	ret = gpio_pin_configure_dt(&config->reset_gpios, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Could not configure reset GPIO (%d)", ret);
		return ret;
	}

	ret = co5300_reset(dev);
	if (ret < 0) {
		LOG_ERR("Failed to reset display: %d", ret);
		return ret;
	}

	data->configured = true;

	/* Initialize display */
	ret = co5300_configure_display(dev);
	if (ret < 0) {
		LOG_ERR("Failed to configure display: %d", ret);
		return ret;
	}

	LOG_DBG("CO5300 display initialized successfully");
	return 0;
}

/* Device driver API*/
static DEVICE_API(display, co5300_api) = {
	.blanking_on = co5300_blanking_on,
	.blanking_off = co5300_blanking_off,
	.write = co5300_write,
	.get_capabilities = co5300_get_capabilities,
	.set_pixel_format = co5300_set_pixel_format,
	.set_orientation = co5300_set_orientation,
};

#define CO5300_INIT(inst)                                               \
	static const struct co5300_config co5300_config_##inst = {          \
		.mspi_bus = DEVICE_DT_GET(DT_INST_BUS(inst)),                   \
		.dev_id = {                                                     \
			.dev_idx = DT_INST_REG_ADDR(inst),                          \
		},                                                              \
		.dc_gpios = GPIO_DT_SPEC_INST_GET_OR(inst, dc_gpios, {}),       \
		.reset_gpios = GPIO_DT_SPEC_INST_GET_OR(inst, reset_gpios, {}), \
		.height = DT_INST_PROP(inst, height),                           \
		.width = DT_INST_PROP(inst, width),                             \
		.pixel_format = DT_INST_PROP(inst, pixel_format),               \
		.write_only = DT_INST_PROP(inst, write_only),                   \
		.orientation = DT_INST_ENUM_IDX(inst, orientation),             \
		.display_invert = DT_INST_PROP(inst, display_invert),           \
		.color_invert = DT_INST_PROP(inst, color_invert),               \
		.sram_size = DT_INST_PROP(inst, sram_size),          			\
		.max_buf_size = DT_INST_PROP(inst, max_buf_size),				\
		.mspi_dev_cfg = MSPI_DEVICE_CONFIG_DT_INST(inst),				\
	};                                                                  \
	static struct co5300_data co5300_data_##inst;                       \
                                                                        \
	DEVICE_DT_INST_DEFINE(inst, &co5300_init, NULL,                     \
		&co5300_data_##inst,                                            \
		&co5300_config_##inst,                                          \
		POST_KERNEL,                                                    \
		CONFIG_DISPLAY_INIT_PRIORITY,                                   \
		&co5300_api);

DT_INST_FOREACH_STATUS_OKAY(CO5300_INIT)
