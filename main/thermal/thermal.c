#include "thermal.h"

#define INTERNAL_OFFSET 5 //degrees C

esp_err_t Thermal_init(DeviceConfig device_config) 
{
    if (device_config.EMC2101) {
        esp_err_t res = EMC2101_init();
        // TODO: Improve this check.
        if (device_config.emc_ideality_factor != 0x00) {
            EMC2101_set_ideality_factor(device_config.emc_ideality_factor);
            EMC2101_set_beta_compensation(device_config.emc_beta_compensation);
        }
        return res;
    }
    if (device_config.EMC2103) {
        return EMC2103_init();
    }
    if (device_config.EMC2302) {
        esp_err_t res = EMC2302_init();
        if (res == ESP_OK) {
            return TMP1075_init();
        }
        return res;
    }

    return ESP_FAIL;
}

//percent is a float between 0.0 and 1.0
esp_err_t Thermal_set_fan_percent(DeviceConfig device_config, float percent)
{
    if (device_config.EMC2101) {
        EMC2101_set_fan_speed(percent);
    }
    if (device_config.EMC2103) {
        EMC2103_set_fan_speed(percent);
    }
    if (device_config.EMC2302) {
        EMC2302_set_fan_speed(0, percent);
        EMC2302_set_fan_speed(1, percent);
    }
    return ESP_OK;
}

uint16_t Thermal_get_fan_speed(DeviceConfig device_config) 
{
    if (device_config.EMC2101) {
        return EMC2101_get_fan_speed();
    }
    if (device_config.EMC2103) {
        return EMC2103_get_fan_speed();
    }
    if (device_config.EMC2302) {
        return EMC2302_get_fan_speed(0);
    }
    return 0;
}

float Thermal_get_chip_temp(GlobalState * GLOBAL_STATE) 
{
    if (!GLOBAL_STATE->ASIC_initalized) {
        return -1;
    }

    if (GLOBAL_STATE->DEVICE_CONFIG.EMC2101) {
        if (GLOBAL_STATE->DEVICE_CONFIG.emc_internal_temp) {
            return EMC2101_get_internal_temp() + INTERNAL_OFFSET;
        } else {
            return EMC2101_get_external_temp();
        }
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.EMC2103) {
        return EMC2103_get_external_temp();
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.EMC2302) {
        return TMP1075_read_temperature(0) + INTERNAL_OFFSET;
    }
    return -1;
}
