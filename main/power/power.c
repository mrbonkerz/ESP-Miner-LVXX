#include "TPS546.h"
#include "INA260.h"
#include "DS4432U.h"

#include "power.h"
#include "vcore.h"

#include <math.h>

#define GPIO_ASIC_ENABLE CONFIG_GPIO_ASIC_ENABLE

#define SUPRA_POWER_OFFSET 5 //Watts
#define GAMMA_POWER_OFFSET 5 //Watts
#define GAMMATURBO_POWER_OFFSET 5 //Watts
#define LV07_POWER_OFFSET 6 //Watts
#define LV08_POWER_OFFSET 18 //Watts

esp_err_t Power_disable(GlobalState * GLOBAL_STATE) {

    switch (GLOBAL_STATE->device_model) {
        case DEVICE_MAX:
        case DEVICE_ULTRA:
        case DEVICE_SUPRA:
            if (GLOBAL_STATE->board_version >= 402 && GLOBAL_STATE->board_version <= 499) {
                // Turn off core voltage
                VCORE_set_voltage(0.0, GLOBAL_STATE);
            } else if (GLOBAL_STATE->board_version == 202 || GLOBAL_STATE->board_version == 203 || GLOBAL_STATE->board_version == 204) {
                gpio_set_level(GPIO_ASIC_ENABLE, 1);
            }
            break;
        case DEVICE_GAMMA:
        case DEVICE_GAMMATURBO:
        case DEVICE_LV07:
        case DEVICE_LV08:
            // Turn off core voltage
            VCORE_set_voltage(0.0, GLOBAL_STATE);
            break;
        default:
    }
    return ESP_OK;

}

float Power_get_power(GlobalState * GLOBAL_STATE) {
    float power = 0.0;
    float current = 0.0;

    switch (GLOBAL_STATE->device_model) {
        case DEVICE_MAX:
        case DEVICE_ULTRA:
        case DEVICE_SUPRA:
            if (GLOBAL_STATE->board_version >= 402 && GLOBAL_STATE->board_version <= 499) {
                current = TPS546_get_iout(0) * 1000.0;
                // calculate regulator power (in milliwatts)
                power = (TPS546_get_vout(0) * current) / 1000.0;
                // The power reading from the TPS546 is only it's output power. So the rest of the Bitaxe power is not accounted for.
                power += SUPRA_POWER_OFFSET; // Add offset for the rest of the Bitaxe power. TODO: this better.
            } else {
                if (INA260_installed() == true) {
                    power = INA260_read_power() / 1000.0;
                }
            }
        
            break;
        case DEVICE_GAMMA:
        case DEVICE_GAMMATURBO:
        case DEVICE_LV07:
                current = TPS546_get_iout(0) * 1000.0;
                // calculate regulator power (in milliwatts)
                power = (TPS546_get_vout(0) * current) / 1000.0;
                // The power reading from the TPS546 is only it's output power. So the rest of the Bitaxe power is not accounted for.
                power += LV07_POWER_OFFSET; // Add offset for the rest of the Bitaxe power. TODO: this better.
            break;
        case DEVICE_LV08:
                // calculate regulator power (in milliwatts)
                power = TPS546_get_vout(0) * TPS546_get_iout(0) + TPS546_get_vout(1) * TPS546_get_iout(1) + TPS546_get_vout(2) * TPS546_get_iout(2);
                // The power reading from the TPS546 is only it's output power. So the rest of the Bitaxe power is not accounted for.
                power += LV08_POWER_OFFSET; // Add offset for the rest of the Bitaxe power. TODO: this better.
            break;
        default:
    }

    return power;
}


float Power_get_input_voltage(GlobalState * GLOBAL_STATE) {

    switch (GLOBAL_STATE->device_model) {
        case DEVICE_MAX:
        case DEVICE_ULTRA:
        case DEVICE_SUPRA:
            if (GLOBAL_STATE->board_version >= 402 && GLOBAL_STATE->board_version <= 499) {
                return TPS546_get_vin(0) * 1000.0;
            } else {
                if (INA260_installed() == true) {
                    return INA260_read_voltage();
                }
            }
        
            break;
        case DEVICE_GAMMA:
        case DEVICE_GAMMATURBO:
        case DEVICE_LV07:
            return TPS546_get_vin(0) * 1000.0;
        case DEVICE_LV08:
            return fmax(fmax(TPS546_get_vin(0), TPS546_get_vin(1)), TPS546_get_vin(2)) * 1000.0;
            break;
        default:
    }

    return 0.0;
}

float Power_get_vreg_temp(GlobalState * GLOBAL_STATE) {

    switch (GLOBAL_STATE->device_model) {
        case DEVICE_MAX:
        case DEVICE_ULTRA:
        case DEVICE_SUPRA:
            if (GLOBAL_STATE->board_version >= 402 && GLOBAL_STATE->board_version <= 499) {
                return TPS546_get_temperature(0);
            } else {
                if (INA260_installed() == true) {
                    return 0.0;
                }
            }
        
            break;
        case DEVICE_GAMMA:
        case DEVICE_GAMMATURBO:
        case DEVICE_LV07:
            return TPS546_get_temperature(0);
        case DEVICE_LV08:
            return fmax(fmax(TPS546_get_temperature(0),TPS546_get_temperature(1)),TPS546_get_temperature(2));
            break;
        default:
    }

    return 0.0;
}