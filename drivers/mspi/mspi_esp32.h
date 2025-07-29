#include <zephyr/drivers/mspi.h>
#include <zephyr/drivers/clock_control.h>

/* ESP-IDF HAL includes */
#include <hal/spi_hal.h>

#ifdef SOC_GDMA_SUPPORTED
#include <hal/gdma_hal.h>
#include <hal/gdma_ll.h>
#endif

#define MSPI_MAX_FREQ 80000000
#define MSPI_TIMEOUT_MS 100
#define MSPI_POLL_INTERVAL_US 10
#define SPI_DUTY_CYCLE_50_PERCENT 128

/* Driver data structure */
struct mspi_esp32_data {
    spi_hal_context_t hal_ctx;
    spi_hal_config_t hal_config;
    spi_hal_dev_config_t dev_config;
    struct mspi_dev_cfg dev_cfg;
    struct k_mutex lock;
    mspi_callback_handler_t callback;
    void *callback_ctx;
    uint32_t callback_mask;
    uint32_t clock_source_hz;
    spi_hal_trans_config_t trans_config;
    lldesc_t dma_desc_tx;
	lldesc_t dma_desc_rx;
#ifdef SOC_GDMA_SUPPORTED
	gdma_hal_context_t hal_gdma;
#endif
};

/* Driver configuration structure */
struct mspi_esp32_config {
    spi_dev_t *reg;
    const struct pinctrl_dev_config *pcfg;
    const struct device *clock_dev;
    clock_control_subsys_t clock_subsys;
    struct mspi_cfg mspicfg;
    uint32_t peripheral_id;
    enum mspi_cpp_mode cpp_mode;
    uint32_t clock_frequency;
    soc_module_clk_t clock_source;
    bool dma_enabled;
    int dma_clk_src;
	int dma_host;
    int max_dma_buf_size;
    bool line_idle_low;
};


/* Helper function to convert Zephyr MSPI mode to ESP32 SPI mode */
static int mspi_mode_to_esp32(enum mspi_cpp_mode mode)
{
    switch (mode) {
        case MSPI_CPP_MODE_0:
            return 0; /* CPOL=0, CPHA=0 */
        case MSPI_CPP_MODE_1:
            return 1; /* CPOL=0, CPHA=1 */
        case MSPI_CPP_MODE_2:
            return 2; /* CPOL=1, CPHA=0 */
        case MSPI_CPP_MODE_3:
            return 3; /* CPOL=1, CPHA=1 */
        default:
            return 0;
    }
}

/* Convert MSPI IO mode to ESP32 SPI line mode */
static spi_line_mode_t get_spi_line_mode(enum mspi_io_mode io_mode) {
    spi_line_mode_t line_mode = {0};

    switch (io_mode) {
        case MSPI_IO_MODE_SINGLE:
            line_mode.cmd_lines = 1;
            line_mode.addr_lines = 1;
            line_mode.data_lines = 1;
            break;
        case MSPI_IO_MODE_DUAL:
        case MSPI_IO_MODE_DUAL_1_1_2:
            line_mode.cmd_lines = 1;
            line_mode.addr_lines = 1;
            line_mode.data_lines = 2;
            break;
        case MSPI_IO_MODE_DUAL_1_2_2:
            line_mode.cmd_lines = 1;
            line_mode.addr_lines = 2;
            line_mode.data_lines = 2;
            break;
        case MSPI_IO_MODE_QUAD:
        case MSPI_IO_MODE_QUAD_1_1_4:
            line_mode.cmd_lines = 1;
            line_mode.addr_lines = 1;
            line_mode.data_lines = 4;
            break;
        case MSPI_IO_MODE_QUAD_1_4_4:
            line_mode.cmd_lines = 1;
            line_mode.addr_lines = 4;
            line_mode.data_lines = 4;
            break;
        case MSPI_IO_MODE_OCTAL:
        case MSPI_IO_MODE_OCTAL_1_1_8:
            line_mode.cmd_lines = 1;
            line_mode.addr_lines = 1;
            line_mode.data_lines = 8;
            break;
        case MSPI_IO_MODE_OCTAL_1_8_8:
            line_mode.cmd_lines = 1;
            line_mode.addr_lines = 8;
            line_mode.data_lines = 8;
            break;
        default:
            line_mode.cmd_lines = 1;
            line_mode.addr_lines = 1;
            line_mode.data_lines = 1;
            break;
    }

    return line_mode;
}
