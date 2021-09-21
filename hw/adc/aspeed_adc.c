/*
 * Aspeed ADC
 *
 * Copyright 2017-2021 IBM Corp.
 *
 * Andrew Jeffery <andrew@aj.id.au>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"
#include "migration/vmstate.h"
#include "hw/adc/aspeed_adc.h"
#include "trace.h"

#define ASPEED_ADC_ENGINE_CTRL                  0x00
#define  ASPEED_ADC_ENGINE_CH_EN_MASK           0xffff0000
#define   ASPEED_ADC_ENGINE_CH_EN(x)            ((BIT(x)) << 16)
#define  ASPEED_ADC_ENGINE_INIT                 BIT(8)
#define  ASPEED_ADC_ENGINE_AUTO_COMP            BIT(5)
#define  ASPEED_ADC_ENGINE_COMP                 BIT(4)
#define  ASPEED_ADC_ENGINE_MODE_MASK            0x0000000e
#define   ASPEED_ADC_ENGINE_MODE_OFF            (0b000 << 1)
#define   ASPEED_ADC_ENGINE_MODE_STANDBY        (0b001 << 1)
#define   ASPEED_ADC_ENGINE_MODE_NORMAL         (0b111 << 1)
#define  ASPEED_ADC_ENGINE_EN                   BIT(0)
#define ASPEED_ADC_HYST_EN                      BIT(31)

#define ASPEED_ADC_L_MASK       ((1 << 10) - 1)
#define ASPEED_ADC_L(x)         ((x) & ASPEED_ADC_L_MASK)
#define ASPEED_ADC_H(x)         (((x) >> 16) & ASPEED_ADC_L_MASK)
#define ASPEED_ADC_LH_MASK      (ASPEED_ADC_L_MASK << 16 | ASPEED_ADC_L_MASK)

static inline uint32_t update_channels(uint32_t current)
{
    return ((((current >> 16) & ASPEED_ADC_L_MASK) + 7) << 16) |
        ((current + 5) & ASPEED_ADC_L_MASK);
}

static bool breaks_threshold(AspeedADCEngineState *s, int ch_off)
{
    const uint32_t a = ASPEED_ADC_L(s->channels[ch_off]);
    const uint32_t a_lower = ASPEED_ADC_L(s->bounds[2 * ch_off]);
    const uint32_t a_upper = ASPEED_ADC_H(s->bounds[2 * ch_off]);
    const uint32_t b = ASPEED_ADC_H(s->channels[ch_off]);
    const uint32_t b_lower = ASPEED_ADC_L(s->bounds[2 * ch_off + 1]);
    const uint32_t b_upper = ASPEED_ADC_H(s->bounds[2 * ch_off + 1]);

    return ((a < a_lower || a > a_upper)) ||
        ((b < b_lower || b > b_upper));
}

static uint32_t read_channel_sample(AspeedADCEngineState *s, int ch_off)
{
    uint32_t ret;

    /* Poor man's sampling */
    ret = s->channels[ch_off];
    s->channels[ch_off] = update_channels(s->channels[ch_off]);

    if (breaks_threshold(s, ch_off)) {
        s->irq_ctrl |= BIT(ch_off);
        qemu_irq_raise(s->irq);
    }

    return ret;
}

#define TO_INDEX(addr, base) (((addr) - (base)) >> 2)

static uint64_t aspeed_adc_engine_read(void *opaque, hwaddr addr,
                                       unsigned int size)
{
    AspeedADCEngineState *s = ASPEED_ADC_ENGINE(opaque);
    uint64_t ret;

    switch (addr) {
    case 0x00:
        ret = s->engine_ctrl;
        break;
    case 0x04:
        ret = s->irq_ctrl;
        break;
    case 0x08:
        ret = s->vga_detect_ctrl;
        break;
    case 0x0c:
        ret = s->adc_clk_ctrl;
        break;
    case 0x10 ... 0x2e:
        ret = read_channel_sample(s, TO_INDEX(addr, 0x10));
        break;
    case 0x30 ... 0x6e:
        ret = s->bounds[TO_INDEX(addr, 0x30)];
        break;
    case 0x70 ... 0xae:
        ret = s->hysteresis[TO_INDEX(addr, 0x70)];
        break;
    case 0xc0:
        ret = s->irq_src;
        break;
    case 0xc4:
        ret = s->comp_trim;
        break;
    default:
        qemu_log_mask(LOG_UNIMP, "%s: addr: 0x%" HWADDR_PRIx ", size: %u\n",
                      __func__, addr, size);
        ret = 0;
        break;
    }

    trace_aspeed_adc_engine_read(s->engine_id, addr, size, ret);

    return ret;
}

static void aspeed_adc_engine_write(void *opaque, hwaddr addr, uint64_t val,
                                    unsigned int size)
{
    AspeedADCEngineState *s = ASPEED_ADC_ENGINE(opaque);
    uint32_t init;

    trace_aspeed_adc_engine_write(s->engine_id, addr, size, val);

    switch (addr) {
    case 0x00:
        init = !!(val & ASPEED_ADC_ENGINE_EN);
        init *= ASPEED_ADC_ENGINE_INIT;

        val &= ~ASPEED_ADC_ENGINE_INIT;
        val |= init;

        val &= ~ASPEED_ADC_ENGINE_AUTO_COMP;
        s->engine_ctrl = val;
        break;
    case 0x04:
        s->irq_ctrl = val;
        break;
    case 0x08:
        s->vga_detect_ctrl = val;
        break;
    case 0x0c:
        s->adc_clk_ctrl = val;
        break;
    case 0x10 ... 0x2e:
        s->channels[TO_INDEX(addr, 0x10)] = (val & ASPEED_ADC_LH_MASK);
        break;
    case 0x30 ... 0x6e:
        s->bounds[TO_INDEX(addr, 0x30)] = (val & ASPEED_ADC_LH_MASK);
        break;
    case 0x70 ... 0xae:
        s->hysteresis[TO_INDEX(addr, 0x70)] =
            (val & (ASPEED_ADC_HYST_EN | ASPEED_ADC_LH_MASK));
        break;
    case 0xc0:
        s->irq_src = (val & 0xffff);
        break;
    case 0xc4:
        s->comp_trim = (val & 0xf);
        break;
    default:
        qemu_log_mask(LOG_UNIMP, "%s: 0x%" HWADDR_PRIx "\n", __func__, addr);
        break;
    }
}

static const MemoryRegionOps aspeed_adc_engine_ops = {
    .read = aspeed_adc_engine_read,
    .write = aspeed_adc_engine_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 4,
    },
};

static void aspeed_adc_engine_reset(DeviceState *dev)
{
    struct AspeedADCEngineState *s = ASPEED_ADC_ENGINE(dev);

    s->engine_ctrl = 0;
    s->irq_ctrl = 0;
    s->vga_detect_ctrl = 0x0000000f;
    s->adc_clk_ctrl = 0x0000000f;
    memset(s->channels, 0, sizeof(s->channels));
    memset(s->bounds, 0, sizeof(s->bounds));
    memset(s->hysteresis, 0, sizeof(s->hysteresis));
    s->irq_src = 0;
    s->comp_trim = 0;
}

static void aspeed_adc_engine_realize(DeviceState *dev, Error **errp)
{
    AspeedADCEngineState *s = ASPEED_ADC_ENGINE(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
    g_autofree char *name = g_strdup_printf(TYPE_ASPEED_ADC_ENGINE ".%d",
                                            s->engine_id);

    assert(s->engine_id < 2);

    sysbus_init_irq(sbd, &s->irq);

    memory_region_init_io(&s->mmio, OBJECT(s), &aspeed_adc_engine_ops, s, name,
                          0x100);

    sysbus_init_mmio(sbd, &s->mmio);
}

static const VMStateDescription vmstate_aspeed_adc_engine = {
    .name = TYPE_ASPEED_ADC,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(engine_ctrl, AspeedADCEngineState),
        VMSTATE_UINT32(irq_ctrl, AspeedADCEngineState),
        VMSTATE_UINT32(vga_detect_ctrl, AspeedADCEngineState),
        VMSTATE_UINT32(adc_clk_ctrl, AspeedADCEngineState),
        VMSTATE_UINT32_ARRAY(channels, AspeedADCEngineState,
                             ASPEED_ADC_NR_CHANNELS / 2),
        VMSTATE_UINT32_ARRAY(bounds, AspeedADCEngineState,
                             ASPEED_ADC_NR_CHANNELS),
        VMSTATE_UINT32_ARRAY(hysteresis, AspeedADCEngineState,
                             ASPEED_ADC_NR_CHANNELS),
        VMSTATE_UINT32(irq_src, AspeedADCEngineState),
        VMSTATE_UINT32(comp_trim, AspeedADCEngineState),
        VMSTATE_END_OF_LIST(),
    }
};

static Property aspeed_adc_engine_properties[] = {
    DEFINE_PROP_UINT32("engine-id", AspeedADCEngineState, engine_id, 0),
    DEFINE_PROP_END_OF_LIST(),
};

static void aspeed_adc_engine_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = aspeed_adc_engine_realize;
    dc->reset = aspeed_adc_engine_reset;
    device_class_set_props(dc, aspeed_adc_engine_properties);
    dc->desc = "Aspeed Analog-to-Digital Engine";
    dc->vmsd = &vmstate_aspeed_adc_engine;
}

static const TypeInfo aspeed_adc_engine_info = {
    .name = TYPE_ASPEED_ADC_ENGINE,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(AspeedADCEngineState),
    .class_init = aspeed_adc_engine_class_init,
};

static void aspeed_adc_instance_init(Object *obj)
{
    AspeedADCState *s = ASPEED_ADC(obj);
    AspeedADCClass *aac = ASPEED_ADC_GET_CLASS(obj);
    int i;

    for (i = 0; i < aac->nr_engines; i++) {
        object_initialize_child(obj, "engine[*]", &s->engines[i],
                                TYPE_ASPEED_ADC_ENGINE);
    }
}

static void aspeed_adc_set_irq(void *opaque, int n, int level)
{
    AspeedADCState *s = opaque;
    AspeedADCClass *aac = ASPEED_ADC_GET_CLASS(s);
    uint32_t pending = 0;
    int i;

    /* TODO: update Global IRQ status register on AST2600 (Need specs) */
    for (i = 0; i < aac->nr_engines; i++) {
        uint32_t irq_status = s->engines[i].irq_ctrl & 0xFF;
        pending |= irq_status << (i * 8);
    }

    qemu_set_irq(s->irq, !!pending);
}

static void aspeed_adc_realize(DeviceState *dev, Error **errp)
{
    AspeedADCState *s = ASPEED_ADC(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
    AspeedADCClass *aac = ASPEED_ADC_GET_CLASS(dev);
    int i;

    qdev_init_gpio_in_named_with_opaque(DEVICE(sbd), aspeed_adc_set_irq,
                                        s, NULL, aac->nr_engines);

    sysbus_init_irq(sbd, &s->irq);

    memory_region_init(&s->mmio, OBJECT(s), TYPE_ASPEED_ADC, 0x1000);

    sysbus_init_mmio(sbd, &s->mmio);

    for (i = 0; i < aac->nr_engines; i++) {
        Object *eng = OBJECT(&s->engines[i]);

        if (!object_property_set_uint(eng, "engine-id", i, errp)) {
            return;
        }
        if (!sysbus_realize(SYS_BUS_DEVICE(eng), errp)) {
            return;
        }
        sysbus_connect_irq(SYS_BUS_DEVICE(eng), 0,
                           qdev_get_gpio_in(DEVICE(sbd), i));
        memory_region_add_subregion(&s->mmio, i * 0x100, &s->engines[i].mmio);
    }
}

static void aspeed_adc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    AspeedADCClass *aac = ASPEED_ADC_CLASS(klass);

    dc->realize = aspeed_adc_realize;
    dc->desc = "Aspeed Analog-to-Digital Converter";
    aac->nr_engines = 1;
}

static const TypeInfo aspeed_adc_info = {
    .name = TYPE_ASPEED_ADC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_init = aspeed_adc_instance_init,
    .instance_size = sizeof(AspeedADCState),
    .class_init = aspeed_adc_class_init,
    .class_size = sizeof(AspeedADCClass),
    .abstract   = true,
};

static const TypeInfo aspeed_2400_adc_info = {
    .name = TYPE_ASPEED_2400_ADC,
    .parent = TYPE_ASPEED_ADC,
};

static const TypeInfo aspeed_2500_adc_info = {
    .name = TYPE_ASPEED_2500_ADC,
    .parent = TYPE_ASPEED_ADC,
};

static void aspeed_2600_adc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    AspeedADCClass *aac = ASPEED_ADC_CLASS(klass);

    dc->desc = "ASPEED 2600 ADC Controller";
    aac->nr_engines = 2;
}

static const TypeInfo aspeed_2600_adc_info = {
    .name = TYPE_ASPEED_2600_ADC,
    .parent = TYPE_ASPEED_ADC,
    .class_init = aspeed_2600_adc_class_init,
};

static void aspeed_adc_register_types(void)
{
    type_register_static(&aspeed_adc_engine_info);
    type_register_static(&aspeed_adc_info);
    type_register_static(&aspeed_2400_adc_info);
    type_register_static(&aspeed_2500_adc_info);
    type_register_static(&aspeed_2600_adc_info);
}

type_init(aspeed_adc_register_types);
