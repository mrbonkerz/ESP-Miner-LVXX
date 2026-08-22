#include "TPS546.h"
#include "INA260.h"

#include "global_state.h"
#include "device_config.h"
#include "power.h"

void Power_get_output(GlobalState * GLOBAL_STATE, float * power_out, float * current_out)
{
    float cur_val = 0.0f;
    float pow_val = 0.0f;

    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546) {
        float iout = TPS546_get_iout(0);
        float vout = TPS546_get_vout(0);
        cur_val = iout * 1000.0f;
        // The power reading from the TPS546 is only it's output power. So the rest of the Bitaxe power is not accounted for.
        pow_val   = vout * iout;
        pow_val  += GLOBAL_STATE->DEVICE_CONFIG.family.power_offset;  // Add offset for the rest of the Bitaxe power. TODO: this better.
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.INA260) {
        cur_val = INA260_read_current();
        pow_val = INA260_read_power() / 1000.0f;
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546_LV08) {
        cur_val = 0.0f;
        pow_val = 0.0f;

        for (int addr = 0; addr < 3; addr++) {
            float iout = TPS546_get_iout(addr);
            float vout = TPS546_get_vout(addr);
            cur_val += iout * 1000.0f;
            pow_val += vout * iout;
        }

        // The power reading from the TPS546 is only output power, so add board offset.
        pow_val += GLOBAL_STATE->DEVICE_CONFIG.family.power_offset;
    }

    *current_out = cur_val;
    *power_out   = pow_val;
}

float Power_get_input_voltage(GlobalState * GLOBAL_STATE)
{
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546) {
        return TPS546_get_vin(0) * 1000.0;
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.INA260) {
        return INA260_read_voltage();
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546_LV08) {
        float vin = TPS546_get_vin(0);

        for (int addr = 1; addr < 3; addr++) {
            float v = TPS546_get_vin(addr);
            if (v > vin)
            vin = v;
        }

        return vin * 1000.0f;
    }
    
    return 0.0;
}

float Power_get_vreg_temp(GlobalState * GLOBAL_STATE)
{
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546) {
        return TPS546_get_temperature(0);
    }
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546_LV08) {
        float temp = TPS546_get_temperature(0);

        for (int addr = 1; addr < 3; addr++) {
            float t = TPS546_get_temperature(addr);
            if (t > temp)
                temp = t;
        }

        return temp;
    }

    return 0.0;
}
