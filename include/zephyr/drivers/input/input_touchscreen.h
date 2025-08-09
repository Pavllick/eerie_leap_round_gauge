/*
 * Copyright (c) 2025 Pavel Maloletkov.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 #pragma once

 #include <zephyr/device.h>
 #include <zephyr/input/input_touch.h>

 #ifdef __cplusplus
 extern "C" {
 #endif

enum touchscreen_dev_cfg_mask {
    TOUCHSCREEN_DEVICE_CONFIG_NONE           = 0,
	TOUCHSCREEN_DEVICE_CONFIG_SCREEN_WIDTH   = BIT(0),
	TOUCHSCREEN_DEVICE_CONFIG_SCREEN_HEIGHT  = BIT(1),
	TOUCHSCREEN_DEVICE_CONFIG_INVERTED_X     = BIT(2),
	TOUCHSCREEN_DEVICE_CONFIG_INVERTED_Y     = BIT(3),
	TOUCHSCREEN_DEVICE_CONFIG_SWAPPED_X_Y    = BIT(4),
	TOUCHSCREEN_DEVICE_CONFIG_ALL            = BIT(5),
};

 /**
  * @brief Register callback for Tearing Effect (TE) GPIO.
  *
  * @param dev Pointer to the device structure for the driver instance.
  * @param handler Callback handler function pointer.
  *
  * @retval 0 If successful.
  */
 int touchscreen_dev_config(struct device *dev, enum touchscreen_dev_cfg_mask param_mask, const struct input_touchscreen_common_config * cfg);

 #ifdef __cplusplus
 }
 #endif
