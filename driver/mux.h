//
//    Copyright (C) 2022 Peter van Tol
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
//
#ifndef __INCLUDE_LITEXCNC_MUX_H__
#define __INCLUDE_LITEXCNC_MUX_H__

#include "litexcnc.h"

// Defines the structure of the mux
typedef struct {
    struct {

        struct {
            /* Velocity in scaled units per second. encoder uses an algorithm that greatly reduces
             * quantization noise as compared to simply differentiating the position output. When 
             * the magnitude of the true velocity is below min-speed-estimate, the velocity output 
             * is 0.
             */
            hal_float_t *scale;
        } pin;

        struct {
            hal_s32_t *mux_type; 
        } param;

    } hal;

} litexcnc_mux_instance_t;

// Defines the MUX, contains a collection of input and output pins
typedef struct {
    int num_instances;
    litexcnc_mux_instance_t *instances;

} litexcnc_mux_t;



// Defines the data-package for sending the settings for a single step generator. The
// order of this package MUST coincide with the order in the MMIO definition.
// - write
#define LITEXCNC_BOARD_MUX_SHARED_INDEX_ENABLE_WRITE_SIZE(litexcnc) (((litexcnc->encoder.num_instances)>>5) + ((litexcnc->encoder.num_instances & 0x1F)?1:0)) *4
#define LITEXCNC_BOARD_MUX_SHARED_RESET_INDEX_PULSE_WRITE_SIZE(litexcnc) (((litexcnc->encoder.num_instances)>>5) + ((litexcnc->encoder.num_instances & 0x1F)?1:0)) *4
#define LITEXCNC_BOARD_MUX_DATA_WRITE_SIZE(litexcnc) LITEXCNC_BOARD_MUX_SHARED_INDEX_ENABLE_WRITE_SIZE(litexcnc) + LITEXCNC_BOARD_MUX_SHARED_RESET_INDEX_PULSE_WRITE_SIZE(litexcnc)
// - read
typedef struct {
    int32_t counts;
} litexcnc_mux_instance_read_data_t;
#define LITEXCNC_BOARD_MUX_SHARED_INDEX_PULSE_READ_SIZE(litexcnc) (((litexcnc->mux.num_instances)>>5) + ((litexcnc->mux.num_instances & 0x1F)?1:0)) *4
#define LITEXCNC_BOARD_MUX_DATA_READ_SIZE(litexcnc) LITEXCNC_BOARD_MUX_SHARED_INDEX_PULSE_READ_SIZE(litexcnc) + sizeof(litexcnc_mux_instance_read_data_t)*litexcnc->mux.num_instances 





#define LITEXCNC_BOARD_GPIO_DATA_READ_SIZE(litexcnc) (((litexcnc->mux.num_input_pins)>>5) + ((litexcnc->mux.num_input_pins & 0x1F)?1:0)) * 4

// Functions for creating, reading and writing GPIO pins
int litexcnc_mux_init(litexcnc_t *litexcnc, json_object *config);
uint8_t litexcnc_mux_prepare_write(litexcnc_t *litexcnc, uint8_t **data);
uint8_t litexcnc_mux_process_read(litexcnc_t *litexcnc, uint8_t** data);

#endif