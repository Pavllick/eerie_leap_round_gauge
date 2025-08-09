#pragma once

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#define CST9220_CHIP_ID             (0x9220)
#define CST9217_CHIP_ID             (0x9217)

#define CST92XX_FW_VER_BLANK        (0xA5A5A5A5)
#define CST92XX_FW_VER_CHECKCODE    (0xCACA0000)
#define CST92XX_MAX_TOUCH_POINTS    (2)

#define CST92XX_STATUS_PRESSED      (0x06)

#define CST92XX_C_CMD_MODE          (0xD101)

#define CST92XX_R_CHECKCODE         (0xD1FC)
#define CST92XX_R_RESOLUTION        (0xD1F8)
#define CST92XX_R_CHIP_TYPE         (0xD204)
#define CST92XX_R_FIRMWARE_VERSION  (0xD208)
#define CST92XX_R_COORDINATES       (0xD000)

#define CST92XX_W_ACK               (0xAB)

#define CST92XX_RESET_DELAY_MS (10)
#define CST92XX_WAIT_DELAY_MS  (50)

struct cst92xx_config {
	struct i2c_dt_spec i2c;
	const struct gpio_dt_spec rst_gpio;
#ifdef CONFIG_INPUT_CST92XX_INTERRUPT
	const struct gpio_dt_spec int_gpio;
#endif
};

/** cst92xx data. */
struct cst92xx_data {
	/** Device pointer. */
	const struct device *dev;
	/** Work queue (for deferred read). */
	struct k_work work;
	uint16_t resolution_x;
	uint16_t resolution_y;

#ifdef CONFIG_INPUT_CST92XX_INTERRUPT
	/** Interrupt GPIO callback. */
	struct gpio_callback int_gpio_cb;
#else
	/** Timer (polling mode). */
	struct k_timer timer;
#endif
};
