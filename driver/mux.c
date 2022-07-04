
#include <stdio.h>
#include <json-c/json.h>

#include "rtapi.h"
#include "rtapi_app.h"

#include "litexcnc.h"
#include "mux.h"


static int litexcnc_mux_in_init(litexcnc_t *litexcnc, json_object *config) {
    
    int r;
    struct json_object *mux;
    struct json_object *mux_pin;
    struct json_object *mux_pin_name;

    char name[HAL_NAME_LEN + 1];
    char name_inverted[HAL_NAME_LEN + 1];
    
    if (json_object_object_get_ex(config, "mux_in", &mux)) {
        litexcnc->mux.num_input_pins = json_object_array_length(mux);
        // Allocate the module-global HAL shared memory
        litexcnc->mux.input_pins = (litexcnc_mux_input_pin_t *)hal_malloc(litexcnc->mux.num_input_pins * sizeof(litexcnc_mux_input_pin_t));
        if (litexcnc->mux.input_pins == NULL) {
            LITEXCNC_ERR_NO_DEVICE("out of memory!\n");
            r = -ENOMEM;
            return r;
        }
        // Create the pins in the HAL
        for (size_t i=0; i<litexcnc->mux.num_input_pins; i++) {
            mux_pin = json_object_array_get_idx(mux, i);
            // Normal pin
            if (json_object_object_get_ex(mux_pin, "name", &mux_pin_name)) {
                rtapi_snprintf(name, sizeof(name), "%s.mux.%s.in", litexcnc->fpga->name, json_object_get_string(mux_pin_name));
                rtapi_snprintf(name_inverted, sizeof(name_inverted), "%s.mux.%s.in-not", litexcnc->fpga->name, json_object_get_string(mux_pin_name));
            } else {
                rtapi_snprintf(name, sizeof(name), "%s.mux.%02d.in", litexcnc->fpga->name, i);
                rtapi_snprintf(name_inverted, sizeof(name_inverted), "%s.mux.%02d.in-not", litexcnc->fpga->name, i);
            }
            r = hal_pin_bit_new(name, HAL_OUT, &(litexcnc->mux.input_pins[i].hal.pin.in), litexcnc->fpga->comp_id);
            if (r < 0) {
                LITEXCNC_ERR_NO_DEVICE("error adding pin '%s', aborting\n", name);
                return r;
            }
            r = hal_pin_bit_new(name_inverted, HAL_OUT, &(litexcnc->mux.input_pins[i].hal.pin.in_not), litexcnc->fpga->comp_id);
            if (r < 0) {
                LITEXCNC_ERR_NO_DEVICE("error adding pin '%s', aborting\n", name);
                return r;
            }
        }
        // Free up the memory
        json_object_put(mux_pin_name);
        json_object_put(mux_pin);
        json_object_put(mux);
    }

    return 0;
    
}

int litexcnc_mux_init(litexcnc_t *litexcnc, json_object *config) {
    int r;

    r = litexcnc_mux_in_init(litexcnc, config);
    if (r<0) {
        return r;
    }
    return r;
}

uint8_t litexcnc_mux_prepare_write(litexcnc_t *litexcnc, uint8_t **data) {
        return 1;
}


uint8_t litexcnc_mux_process_read(litexcnc_t *litexcnc, uint8_t** data) {

    if (litexcnc->mux.num_input_pins == 0) {
        return 1;
    }

    // Process all the bytes
    uint8_t mask = 0x80;
    for (size_t i=LITEXCNC_BOARD_MUX_DATA_READ_SIZE(litexcnc)*8; i>0; i--) {
        // The counter i can have a value outside the range of possible pins. We only
        // should add data to existing pins
        if (i <= litexcnc->mux.num_input_pins) {
            if (*(*data) & mask) {
                // MUX active
                *(litexcnc->mux.input_pins[i-1].hal.pin.in) = 1;
                *(litexcnc->mux.input_pins[i-1].hal.pin.in_not) = 0;
            } else {
                // MUX inactive
                *(litexcnc->mux.input_pins[i-1].hal.pin.in) = 0;
                *(litexcnc->mux.input_pins[i-1].hal.pin.in_not) = 1;
            }
        }
        // Modify the mask for the next. When the mask is zero (happens in case of a 
        // roll-over), we should proceed to the next byte and reset the mask.
        mask >>= 1;
        if (!mask) {
            mask = 0x80;  // Reset the mask
            (*data)++; // Proceed the buffer to the next element
        }
    }
}
