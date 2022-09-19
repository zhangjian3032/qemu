/*
 * QEMU Host Power device
 *
 * Copyright (C) Jian Zhang <zhangjian.3032@bytedance.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef HW_MISC_HOST_POWER_H
#define HW_MISC_HOST_POWER_H

#include "qom/object.h"
#include "hw/qdev-core.h"

#define TYPE_HOST_POWER "host-power"

struct HostPowerState {
    /* Private */
    DeviceState parent_obj;
    /* Public */

    qemu_irq power_button;
    qemu_irq power_good;

    /* Properties */
    bool power_status;
    char *description;
};
typedef struct HostPowerState HostPowerState;
DECLARE_INSTANCE_CHECKER(HostPowerState, HOST_POWER, TYPE_HOST_POWER)

/**
 * host_power_create_simple: Create and realize a  device
 * @parentobj: the parent object
 *
 * Create the device state structure, initialize it, and
 * drop the reference to it (the device is realized).
 *
 */
HostPowerState *host_power_create_simple(Object *parentobj);

#endif /* HW_MISC_HOST_POWER_H */
