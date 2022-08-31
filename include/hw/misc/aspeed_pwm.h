/*
 * ASPEED PWM Controller
 *
 * Copyright (C) 2017-2021 IBM Corp.
 *
 * This code is licensed under the GPL version 2 or later.  See
 * the COPYING file in the top-level directory.
 */

#ifndef ASPEED_PWM_H
#define ASPEED_PWM_H

#include "hw/sysbus.h"

#define TYPE_ASPEED_PWM "aspeed.pwm"
#define ASPEED_PWM(obj) OBJECT_CHECK(AspeedPWMState, (obj), TYPE_ASPEED_PWM)

#define ASPEED_PWM_NR_REGS (0x7C >> 2)

typedef struct AspeedPWMState {
    /* <private> */
    SysBusDevice parent;

    /*< public >*/
    MemoryRegion iomem;
    qemu_irq irq;

    uint32_t regs[ASPEED_PWM_NR_REGS];
} AspeedPWMState;

#endif /* _ASPEED_PWM_H_ */
