zephyr_sources_ifdef(
    CONFIG_MSPI_ESP32
    ${ZEPHYR_HAL_ESPRESSIF_MODULE_DIR}/components/hal/spi_hal.c
    ${ZEPHYR_HAL_ESPRESSIF_MODULE_DIR}/components/hal/spi_hal_iram.c
    ${ZEPHYR_HAL_ESPRESSIF_MODULE_DIR}/components/soc/lldesc.c
    ${ZEPHYR_HAL_ESPRESSIF_MODULE_DIR}/components/hal/gdma_hal.c
)

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_MSPI_ESP32 mspi_esp32.c)
