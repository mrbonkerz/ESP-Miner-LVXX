#include "TPS546.h"
#include "INA260.h"
#include "DS4432U.h"

#include "power.h"

#include <math.h>

float Power_get_current(GlobalState * GLOBAL_STATE)
{
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546 || GLOBAL_STATE->DEVICE_CONFIG.TPS546_1) {
        return TPS546_get_iout(0) * 1000.0;
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546_3) {
        return fmax(fmax(TPS546_get_iout(0), TPS546_get_iout(1)), TPS546_get_iout(2)) * 1000.0;
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.INA260) {
        // TODO: Does this check still need to happen?
        if (INA260_installed() == true) {
            return INA260_read_current();
        }
    }

    return 0.0;
}

float Power_get_power(GlobalState * GLOBAL_STATE)
{
    float power = 0.0;
    float current = 0.0;

    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546 || GLOBAL_STATE->DEVICE_CONFIG.TPS546_1) {
        current = TPS546_get_iout(0) * 1000.0;
        // calculate regulator power (in milliwatts)
        power = (TPS546_get_vout(0) * current) / 1000.0;
        // The power reading from the TPS546 is only it's output power. So the rest of the Bitaxe power is not accounted for.
        power += GLOBAL_STATE->DEVICE_CONFIG.family.power_offset; // Add offset for the rest of the Bitaxe power. TODO: this better.
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546_3) {
        // calculate regulator power (in milliwatts)
        power = TPS546_get_vout(0) * TPS546_get_iout(0) + TPS546_get_vout(1) * TPS546_get_iout(1) + TPS546_get_vout(2) * TPS546_get_iout(2);
        // The power reading from the TPS546 is only it's output power. So the rest of the Bitaxe power is not accounted for.
        power += GLOBAL_STATE->DEVICE_CONFIG.family.power_offset; // Add offset for the rest of the Bitaxe power. TODO: this better.
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.INA260) {
        // TODO: Does this check still need to happen?
        if (INA260_installed() == true) {
            power = INA260_read_power() / 1000.0;
        }
    }    

    return power;
}

float Power_get_input_voltage(GlobalState * GLOBAL_STATE)
{
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546 || GLOBAL_STATE->DEVICE_CONFIG.TPS546_1) {
        return TPS546_get_vin(0) * 1000.0;
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546_3) {
        return fmax(fmax(TPS546_get_vin(0), TPS546_get_vin(1)), TPS546_get_vin(2)) * 1000.0;
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.INA260) {
        // TODO: Does this check still need to happen?
        if (INA260_installed() == true) {
            return INA260_read_voltage();
        }
    }
    
    return 0.0;
}

float Power_get_vreg_temp(GlobalState * GLOBAL_STATE)
{
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546 || GLOBAL_STATE->DEVICE_CONFIG.TPS546_1) {
        return TPS546_get_temperature(0);
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546_3) {
        return fmax(fmax(TPS546_get_temperature(0),TPS546_get_temperature(1)),TPS546_get_temperature(2));
    }

    return 0.0;
}
