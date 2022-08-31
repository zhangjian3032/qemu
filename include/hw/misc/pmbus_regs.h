/*
 * Common defines and structures for PMBus devices
 *
 * Extracted from Linux drivers/hwmon/pmbus/pmbus.h
 *
 * Copyright (c) 2010, 2011 Ericsson AB.
 * Copyright (c) 2012 Guenter Roeck
 *
 * This code is licensed under the GPL version 2 or later. See the
 * COPYING file in the top-level directory.
 */

#ifndef PMBUS_REGS_H
#define PMBUS_REGS_H

/*
 * Registers
 */
enum PmbusRegs {
        PMBUS_PAGE                      = 0x00,
        PMBUS_OPERATION                 = 0x01,
        PMBUS_ON_OFF_CONFIG             = 0x02,
        PMBUS_CLEAR_FAULTS              = 0x03,
        PMBUS_PHASE                     = 0x04,

        PMBUS_CAPABILITY                = 0x19,
        PMBUS_QUERY                     = 0x1A,

        PMBUS_VOUT_MODE                 = 0x20,
        PMBUS_VOUT_COMMAND              = 0x21,
        PMBUS_VOUT_TRIM                 = 0x22,
        PMBUS_VOUT_CAL_OFFSET           = 0x23,
        PMBUS_VOUT_MAX                  = 0x24,
        PMBUS_VOUT_MARGIN_HIGH          = 0x25,
        PMBUS_VOUT_MARGIN_LOW           = 0x26,
        PMBUS_VOUT_TRANSITION_RATE      = 0x27,
        PMBUS_VOUT_DROOP                = 0x28,
        PMBUS_VOUT_SCALE_LOOP           = 0x29,
        PMBUS_VOUT_SCALE_MONITOR        = 0x2A,

        PMBUS_COEFFICIENTS              = 0x30,
        PMBUS_POUT_MAX                  = 0x31,

        PMBUS_FAN_CONFIG_12             = 0x3A,
        PMBUS_FAN_COMMAND_1             = 0x3B,
        PMBUS_FAN_COMMAND_2             = 0x3C,
        PMBUS_FAN_CONFIG_34             = 0x3D,
        PMBUS_FAN_COMMAND_3             = 0x3E,
        PMBUS_FAN_COMMAND_4             = 0x3F,

        PMBUS_VOUT_OV_FAULT_LIMIT       = 0x40,
        PMBUS_VOUT_OV_FAULT_RESPONSE    = 0x41,
        PMBUS_VOUT_OV_WARN_LIMIT        = 0x42,
        PMBUS_VOUT_UV_WARN_LIMIT        = 0x43,
        PMBUS_VOUT_UV_FAULT_LIMIT       = 0x44,
        PMBUS_VOUT_UV_FAULT_RESPONSE    = 0x45,
        PMBUS_IOUT_OC_FAULT_LIMIT       = 0x46,
        PMBUS_IOUT_OC_FAULT_RESPONSE    = 0x47,
        PMBUS_IOUT_OC_LV_FAULT_LIMIT    = 0x48,
        PMBUS_IOUT_OC_LV_FAULT_RESPONSE = 0x49,
        PMBUS_IOUT_OC_WARN_LIMIT        = 0x4A,
        PMBUS_IOUT_UC_FAULT_LIMIT       = 0x4B,
        PMBUS_IOUT_UC_FAULT_RESPONSE    = 0x4C,

        PMBUS_OT_FAULT_LIMIT            = 0x4F,
        PMBUS_OT_FAULT_RESPONSE         = 0x50,
        PMBUS_OT_WARN_LIMIT             = 0x51,
        PMBUS_UT_WARN_LIMIT             = 0x52,
        PMBUS_UT_FAULT_LIMIT            = 0x53,
        PMBUS_UT_FAULT_RESPONSE         = 0x54,
        PMBUS_VIN_OV_FAULT_LIMIT        = 0x55,
        PMBUS_VIN_OV_FAULT_RESPONSE     = 0x56,
        PMBUS_VIN_OV_WARN_LIMIT         = 0x57,
        PMBUS_VIN_UV_WARN_LIMIT         = 0x58,
        PMBUS_VIN_UV_FAULT_LIMIT        = 0x59,

        PMBUS_IIN_OC_FAULT_LIMIT        = 0x5B,
        PMBUS_IIN_OC_WARN_LIMIT         = 0x5D,

        PMBUS_POUT_OP_FAULT_LIMIT       = 0x68,
        PMBUS_POUT_OP_WARN_LIMIT        = 0x6A,
        PMBUS_PIN_OP_WARN_LIMIT         = 0x6B,

        PMBUS_STATUS_BYTE               = 0x78,
        PMBUS_STATUS_WORD               = 0x79,
        PMBUS_STATUS_VOUT               = 0x7A,
        PMBUS_STATUS_IOUT               = 0x7B,
        PMBUS_STATUS_INPUT              = 0x7C,
        PMBUS_STATUS_TEMPERATURE        = 0x7D,
        PMBUS_STATUS_CML                = 0x7E,
        PMBUS_STATUS_OTHER              = 0x7F,
        PMBUS_STATUS_MFR_SPECIFIC       = 0x80,
        PMBUS_STATUS_FAN_12             = 0x81,
        PMBUS_STATUS_FAN_34             = 0x82,

        PMBUS_READ_VIN                  = 0x88,
        PMBUS_READ_IIN                  = 0x89,
        PMBUS_READ_VCAP                 = 0x8A,
        PMBUS_READ_VOUT                 = 0x8B,
        PMBUS_READ_IOUT                 = 0x8C,
        PMBUS_READ_TEMPERATURE_1        = 0x8D,
        PMBUS_READ_TEMPERATURE_2        = 0x8E,
        PMBUS_READ_TEMPERATURE_3        = 0x8F,
        PMBUS_READ_FAN_SPEED_1          = 0x90,
        PMBUS_READ_FAN_SPEED_2          = 0x91,
        PMBUS_READ_FAN_SPEED_3          = 0x92,
        PMBUS_READ_FAN_SPEED_4          = 0x93,
        PMBUS_READ_DUTY_CYCLE           = 0x94,
        PMBUS_READ_FREQUENCY            = 0x95,
        PMBUS_READ_POUT                 = 0x96,
        PMBUS_READ_PIN                  = 0x97,

        PMBUS_REVISION                  = 0x98,
        PMBUS_MFR_ID                    = 0x99,
        PMBUS_MFR_MODEL                 = 0x9A,
        PMBUS_MFR_REVISION              = 0x9B,
        PMBUS_MFR_LOCATION              = 0x9C,
        PMBUS_MFR_DATE                  = 0x9D,
        PMBUS_MFR_SERIAL                = 0x9E,
};

#endif
