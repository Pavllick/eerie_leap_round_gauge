/*
 * Copyright (c) 2025 Pavel Maloletkov.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register callback for Tearing Effect (TE) GPIO.
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @param handler Callback handler function pointer.
 *
 * @retval 0 If successful.
 */
int co5300_te_gpio_register_callback(const struct device *dev, gpio_callback_handler_t handler);

#ifdef __cplusplus
}
#endif
