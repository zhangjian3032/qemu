/*
 * QEMU single Host Power device
 *
 * Copyright (C) 2022 Jian Zhang <zhangjian.3032@bytedance.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "migration/vmstate.h"
#include "hw/qdev-properties.h"
#include "hw/irq.h"
#include "hw/misc/host_power.h"
#include "trace.h"

static void power_control(HostPowerState *s, bool on)
{
    if (on)
    {
        qemu_set_irq(s->power_good, 1);
    }
    else
    {
        qemu_set_irq(s->power_good, 0);
    }
    s->power_status = on;
}

static void power_button_handler(void *opaque, int line, int new_state)
{
    HostPowerState *s = HOST_POWER(opaque);

    assert(line == 0);

    if (new_state == 0)
    {
        // when falling edge, reverse the power status
        if (s->power_status == 0)
        {
            power_control(s, 1);
        }
        else
        {
            power_control(s, 0);
        }
    }
}

static void host_power_reset(DeviceState *dev)
{
    HostPowerState *s = HOST_POWER(dev);
    s->power_status = false;
}

static const VMStateDescription vmstate_host_power = {
    .name = TYPE_HOST_POWER,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_END_OF_LIST()
    }
};

static void host_power_realize(DeviceState *dev, Error **errp)
{
    HostPowerState *s = HOST_POWER(dev);
    s->power_status = false;

    // init a button input gpio
    qdev_init_gpio_in_named(dev, power_button_handler, "button", 1);

    // init a power as output
    qdev_init_gpio_out_named(dev, &(s->power_good), "power-good", 1);
}

static void host_power_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->desc = "Host Power";
    dc->vmsd = &vmstate_host_power;
    dc->reset = host_power_reset;
    dc->realize = host_power_realize;
    set_bit(DEVICE_CATEGORY_DISPLAY, dc->categories);
}

static const TypeInfo host_power_info = {
    .name = TYPE_HOST_POWER,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(HostPowerState),
    .class_init = host_power_class_init
};

static void host_power_register_types(void)
{
    type_register_static(&host_power_info);
}

type_init(host_power_register_types)

HostPowerState *host_power_create_simple(Object *parentobj)
{
    static const char *name = "host-power";
    DeviceState *dev;

    dev = qdev_new(TYPE_HOST_POWER);

    object_property_add_child(parentobj, name, OBJECT(dev));
    qdev_realize_and_unref(dev, NULL, &error_fatal);

    return HOST_POWER(dev);
}