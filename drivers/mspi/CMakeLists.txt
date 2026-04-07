if(CONFIG_MSPI_ESP32)
    zephyr_sources(
        ${ZEPHYR_HAL_ESPRESSIF_MODULE_DIR}/components/esp_hal_gpspi/spi_hal.c
        ${ZEPHYR_HAL_ESPRESSIF_MODULE_DIR}/components/esp_hal_gpspi/spi_hal_iram.c
    )

    # lldesc.c is only needed for the legacy integrated-DMA path.
    # SOC_GDMA_SUPPORTED is a Kconfig symbol set by the SoC Kconfig.
    if(NOT CONFIG_SOC_GDMA)
        zephyr_sources(
            ${ZEPHYR_HAL_ESPRESSIF_MODULE_DIR}/components/soc/lldesc.c
        )
    endif()
endif()

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_MSPI_ESP32 mspi_esp32.c)
