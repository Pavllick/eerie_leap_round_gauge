#include <string>

#include <zephyr/logging/log.h>
#ifdef CONFIG_SDMMC_SUBSYS
#include <zephyr/sd/sd.h>
#endif
#include "dt_fs.h"

namespace eerie_leap::subsys::device_tree {

LOG_MODULE_REGISTER(dt_fs_logger);

std::optional<fs_mount_t> DtFs::int_fs_mp_ = std::nullopt;
std::optional<fs_mount_t> DtFs::sd_fs_mp_ = std::nullopt;
#ifdef CONFIG_SDMMC_SUBSYS
sdhc_host_props DtFs::sd_host_props_;
#endif
uint32_t DtFs::sd_relative_addr_ = 0;
const char* DtFs::sd_disk_name_ = nullptr;

void DtFs::InitInternalFs() {
#if DT_HAS_ALIAS(intfs0)
    int_fs_mp_ = std::make_optional<fs_mount_t>(FS_FSTAB_ENTRY(INT_FS_NODE));
    LOG_INF("Internal File System initialized.");
#endif
}

void DtFs::InitSdFs() {
#if DT_HAS_ALIAS(sdhc0) && DT_HAS_ALIAS(sdfs0) && CONFIG_SDMMC_SUBSYS
    sd_fs_mp_ = std::make_optional<fs_mount_t>(FS_FSTAB_ENTRY(SD_FS_NODE));
    sd_fs_mp_.value().storage_dev = (void *)SD_DEV;

    int ret = sdhc_get_host_props(SD_DEV, &sd_host_props_);
    if (ret) {
        LOG_ERR("SDHC get host props failed with error %d", ret);
        return;
    }

    LOG_INF("SD File System initialized.");

	const char* disk_names[] = {DT_FOREACH_CHILD_SEP(SDHC_NODE, SDHC_DISK_NAME, (,))};
	if(sizeof(disk_names) > 0)
		sd_disk_name_ = disk_names[0];
#endif
}

#ifdef CONFIG_SDMMC_SUBSYS
/*
 * Requests card to publish a new relative card address, and move from
 * identification to data mode
 */
int DtFs::SdmmcRequestRca(const struct device* dev) {
	struct sdhc_command cmd;
	int ret;

	cmd.opcode = SD_SEND_RELATIVE_ADDR;
	cmd.arg = 0;
	cmd.response_type = SD_RSP_TYPE_R6;
	cmd.retries = CONFIG_SD_CMD_RETRIES;
	cmd.timeout_ms = CONFIG_SD_CMD_TIMEOUT;
	/* Issue CMD3 until card responds with nonzero RCA */
	do {
		ret = sdhc_request(dev, &cmd, NULL);
		if(ret) {
			LOG_DBG("CMD3 failed");
			return ret;
		}

		/* Card RCA is in upper 16 bits of response */
		sd_relative_addr_ = ((cmd.response[0U] & 0xFFFF0000) >> 16U);
	} while (sd_relative_addr_ == 0U);

	LOG_DBG("Card relative addr: %d", sd_relative_addr_);
	return 0;
}

bool DtFs::SdmmcReadStatus(const struct device* dev) {
	struct sdhc_command cmd;
	int ret;

	cmd.opcode = SD_SEND_STATUS;
	cmd.arg = 0;
	if (!sd_host_props_.is_spi) {
        if(sd_relative_addr_ == 0U) {
            ret = SdmmcRequestRca(dev);
            if(ret)
                return false;
        }

		cmd.arg = (sd_relative_addr_ << 16U);
	}
	cmd.response_type = (SD_RSP_TYPE_R1 | SD_SPI_RSP_TYPE_R2);
	cmd.retries = CONFIG_SD_CMD_RETRIES;
	cmd.timeout_ms = CONFIG_SD_CMD_TIMEOUT;

	ret = sdhc_request(dev, &cmd, NULL);
	if (ret) {
		return false;
	}

    return true;
}
#endif

bool DtFs::IsSdCardPresent() {
#if DT_HAS_ALIAS(sdhc0) && DT_HAS_ALIAS(sdfs0) && CONFIG_SDMMC_SUBSYS
    // NOTE: SPI SDHC driver does not support card detection.
    if(sd_host_props_.is_spi)
        return SdmmcReadStatus(SD_DEV);

    return sd_is_card_present(SD_DEV);
    // return sdhc_card_present(SD_DEV) == 1;
#endif

    return false;
}

} // namespace eerie_leap::subsys::device_tree
