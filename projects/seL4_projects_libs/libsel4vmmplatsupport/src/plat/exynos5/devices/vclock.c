/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdlib.h>
#include <string.h>

#include <sel4vm/guest_vm.h>
#include <sel4vmmplatsupport/device.h>
#include <sel4vmmplatsupport/arch/ac_device.h>
#include <sel4vmmplatsupport/plat/device_map.h>
#include <sel4vmmplatsupport/plat/vclock.h>

#define CLOCK_REG_DEFN(r, o, s, b) { .bank = CLKREGS_##r, .offset = o, .shift = s, .bits = b }
#define CLK_DEFN(c) { .nregs = sizeof(c)/sizeof(*c), .regs = &c[0] }
#define CLK_DEFN_NONE() { .nregs = 0, .regs = NULL }

struct clock_reg {
    int bank;
    int offset;
    int shift;
    int bits;
};

struct clock_data {
    int nregs;
    struct clock_reg *regs;
};

struct clock_reg sclkmpll_data[] = {
    CLOCK_REG_DEFN(CORE, 0x000, 0, 32), /* LOCK */
    CLOCK_REG_DEFN(CORE, 0x100, 0, 32), /* CON0 */
    CLOCK_REG_DEFN(CORE, 0x104, 0, 32), /* CON1 */
};
struct clock_reg sclkbpll_data[] = {
    CLOCK_REG_DEFN(CDREX, 0x010, 0, 32), /* LOCK */
    CLOCK_REG_DEFN(CDREX, 0x110, 0, 32), /* CON0 */
    CLOCK_REG_DEFN(CDREX, 0x114, 0, 32), /* CON1 */
};
struct clock_reg sclkcpll_data[] = {
    CLOCK_REG_DEFN(TOP, 0x020, 0, 32), /* LOCK */
    CLOCK_REG_DEFN(TOP, 0x120, 0, 32), /* CON0 */
    CLOCK_REG_DEFN(TOP, 0x124, 0, 32), /* CON1 */
    CLOCK_REG_DEFN(TOP, 0x128, 0, 32), /* CON2 */
};
struct clock_reg sclkgpll_data[] = {
    CLOCK_REG_DEFN(TOP, 0x050, 0, 32), /* LOCK */
    CLOCK_REG_DEFN(TOP, 0x150, 0, 32), /* CON0 */
    CLOCK_REG_DEFN(TOP, 0x154, 0, 32), /* CON1 */
};
struct clock_reg sclkepll_data[] = {
    CLOCK_REG_DEFN(TOP, 0x030, 0, 32), /* LOCK */
    CLOCK_REG_DEFN(TOP, 0x130, 0, 32), /* CON0 */
    CLOCK_REG_DEFN(TOP, 0x134, 0, 32), /* CON1 */
    CLOCK_REG_DEFN(TOP, 0x138, 0, 32), /* CON2 */
};
struct clock_reg sclkvpll_data[] = {
    CLOCK_REG_DEFN(TOP, 0x040, 0, 32), /* LOCK */
    CLOCK_REG_DEFN(TOP, 0x140, 0, 32), /* CON0 */
    CLOCK_REG_DEFN(TOP, 0x144, 0, 32), /* CON1 */
    CLOCK_REG_DEFN(TOP, 0x148, 0, 32), /* CON2 */
};

struct clock_reg uart0_data[] = {
    CLOCK_REG_DEFN(TOP, 0x250, 0, 4), /* SEL  */
    CLOCK_REG_DEFN(TOP, 0x350, 0, 1), /* MASK */
    CLOCK_REG_DEFN(TOP, 0x558, 0, 4), /* DIV  */
    CLOCK_REG_DEFN(TOP, 0x658, 0, 1), /* DIVSTAT */
    CLOCK_REG_DEFN(TOP, 0x950, 0, 1), /* GATE */
};
struct clock_reg uart1_data[] = {
    CLOCK_REG_DEFN(TOP, 0x250, 4, 4), /* SEL */
    CLOCK_REG_DEFN(TOP, 0x350, 4, 1), /* MASK */
    CLOCK_REG_DEFN(TOP, 0x558, 4, 4), /* DIV  */
    CLOCK_REG_DEFN(TOP, 0x658, 4, 1), /* DIVSTAT */
    CLOCK_REG_DEFN(TOP, 0x950, 1, 1), /* GATE */
};
struct clock_reg uart2_data[] = {
    CLOCK_REG_DEFN(TOP, 0x250, 8, 4), /* SEL */
    CLOCK_REG_DEFN(TOP, 0x350, 8, 1), /* MASK */
    CLOCK_REG_DEFN(TOP, 0x558, 8, 4), /* DIV  */
    CLOCK_REG_DEFN(TOP, 0x658, 8, 1), /* DIVSTAT */
    CLOCK_REG_DEFN(TOP, 0x950, 2, 1), /* GATE */
};
struct clock_reg uart3_data[] = {
    CLOCK_REG_DEFN(TOP, 0x250, 12, 4), /* SEL */
    CLOCK_REG_DEFN(TOP, 0x350, 12, 1), /* MASK */
    CLOCK_REG_DEFN(TOP, 0x558, 12, 4), /* DIV  */
    CLOCK_REG_DEFN(TOP, 0x658, 12, 1), /* DIVSTAT */
    CLOCK_REG_DEFN(TOP, 0x950,  3, 1), /* GATE */
};

struct clock_reg spi0_data[] = {
    CLOCK_REG_DEFN(TOP, 0x254, 16, 4), /* SEL */
    CLOCK_REG_DEFN(TOP, 0x354, 16, 1), /* MASK */
    CLOCK_REG_DEFN(TOP, 0x55C,  0, 4), /* DIV  */
    CLOCK_REG_DEFN(TOP, 0x55C,  8, 8), /* DIV_PRE  */
    CLOCK_REG_DEFN(TOP, 0x65C,  0, 1), /* DIVSTAT */
    CLOCK_REG_DEFN(TOP, 0x65C,  8, 1), /* DIVSTAT_PRE */
    CLOCK_REG_DEFN(TOP, 0x950, 16, 1), /* GATE */
};
struct clock_reg spi1_data[] = {
    CLOCK_REG_DEFN(TOP, 0x254, 20, 4), /* SEL */
    CLOCK_REG_DEFN(TOP, 0x354, 20, 1), /* MASK */
    CLOCK_REG_DEFN(TOP, 0x55C, 16, 4), /* DIV  */
    CLOCK_REG_DEFN(TOP, 0x55C, 24, 8), /* DIV_PRE  */
    CLOCK_REG_DEFN(TOP, 0x65C, 16, 1), /* DIVSTAT */
    CLOCK_REG_DEFN(TOP, 0x65C, 24, 1), /* DIVSTAT_PRE */
    CLOCK_REG_DEFN(TOP, 0x950, 17, 1), /* GATE */
};
struct clock_reg spi2_data[] = {
    CLOCK_REG_DEFN(TOP, 0x254, 24, 4), /* SEL */
    CLOCK_REG_DEFN(TOP, 0x354, 24, 1), /* MASK */
    CLOCK_REG_DEFN(TOP, 0x560,  0, 4), /* DIV  */
    CLOCK_REG_DEFN(TOP, 0x560,  8, 8), /* DIV_PRE  */
    CLOCK_REG_DEFN(TOP, 0x660,  0, 1), /* DIVSTAT */
    CLOCK_REG_DEFN(TOP, 0x660,  8, 1), /* DIVSTAT_PRE */
    CLOCK_REG_DEFN(TOP, 0x950, 18, 1), /* GATE */
};

struct clock_reg i2c0_data[] = {
    CLOCK_REG_DEFN(TOP, 0x950, 6, 1), /* GATE */
};
struct clock_reg i2c1_data[] = {
    CLOCK_REG_DEFN(TOP, 0x950, 7, 1), /* GATE */
};
struct clock_reg i2c2_data[] = {
    CLOCK_REG_DEFN(TOP, 0x950, 8, 1), /* GATE */
};
struct clock_reg i2c3_data[] = {
    CLOCK_REG_DEFN(TOP, 0x950, 9, 1), /* GATE */
};
struct clock_reg i2c4_data[] = {
    CLOCK_REG_DEFN(TOP, 0x950, 10, 1), /* GATE */
};
struct clock_reg i2c5_data[] = {
    CLOCK_REG_DEFN(TOP, 0x950, 11, 1), /* GATE */
};
struct clock_reg i2c6_data[] = {
    CLOCK_REG_DEFN(TOP, 0x950, 12, 1), /* GATE */
};
struct clock_reg i2c7_data[] = {
    CLOCK_REG_DEFN(TOP, 0x950, 13, 1), /* GATE */
};

struct clock_reg mmc0_data[] = {
    CLOCK_REG_DEFN(TOP, 0x244, 0, 4), /* SEL */
    CLOCK_REG_DEFN(TOP, 0x340, 0, 1), /* MASK */
    CLOCK_REG_DEFN(TOP, 0x54C, 0, 4), /* DIV */
    CLOCK_REG_DEFN(TOP, 0x54C, 8, 8), /* DIV_PRE */
    CLOCK_REG_DEFN(TOP, 0x64C, 0, 1), /* DIVSTAT */
    CLOCK_REG_DEFN(TOP, 0x64C, 8, 1), /* DIVSTAT_PRE */
    CLOCK_REG_DEFN(TOP, 0x944, 12, 1), /* GATE */
};
struct clock_reg mmc1_data[] = {
    CLOCK_REG_DEFN(TOP, 0x244, 4, 4), /* SEL */
    CLOCK_REG_DEFN(TOP, 0x340, 4, 1), /* MASK */
    CLOCK_REG_DEFN(TOP, 0x54C, 16, 4), /* DIV */
    CLOCK_REG_DEFN(TOP, 0x54C, 24, 8), /* DIV_PRE */
    CLOCK_REG_DEFN(TOP, 0x64C, 16, 1), /* DIVSTAT */
    CLOCK_REG_DEFN(TOP, 0x64C, 24, 1), /* DIVSTAT_PRE */
    CLOCK_REG_DEFN(TOP, 0x944, 13, 1), /* GATE */
};
struct clock_reg mmc2_data[] = {
    CLOCK_REG_DEFN(TOP, 0x244, 8, 4), /* SEL */
    CLOCK_REG_DEFN(TOP, 0x340, 8, 1), /* MASK */
    CLOCK_REG_DEFN(TOP, 0x550, 0, 4), /* DIV */
    CLOCK_REG_DEFN(TOP, 0x550, 8, 8), /* DIV_PRE */
    CLOCK_REG_DEFN(TOP, 0x650, 0, 1), /* DIVSTAT */
    CLOCK_REG_DEFN(TOP, 0x650, 8, 1), /* DIVSTAT_PRE */
    CLOCK_REG_DEFN(TOP, 0x944, 14, 1), /* GATE */
};
struct clock_reg mmc3_data[] = {
    CLOCK_REG_DEFN(TOP, 0x244, 12, 4), /* SEL */
    CLOCK_REG_DEFN(TOP, 0x340, 12, 1), /* MASK */
    CLOCK_REG_DEFN(TOP, 0x550, 16, 4), /* DIV */
    CLOCK_REG_DEFN(TOP, 0x550, 24, 8), /* DIV_PRE */
    CLOCK_REG_DEFN(TOP, 0x650, 16, 1), /* DIVSTAT */
    CLOCK_REG_DEFN(TOP, 0x650, 24, 1), /* DIVSTAT_PRE */
    CLOCK_REG_DEFN(TOP, 0x944, 15, 1), /* GATE */
};

struct clock_data clock_data[] = {
    [CLK_MASTER  ] = CLK_DEFN_NONE(),
    [CLK_SCLKMPLL] = CLK_DEFN(sclkmpll_data),
    [CLK_SCLKBPLL] = CLK_DEFN(sclkbpll_data),
    [CLK_SCLKCPLL] = CLK_DEFN(sclkcpll_data),
    [CLK_SCLKGPLL] = CLK_DEFN(sclkgpll_data),
    [CLK_SCLKEPLL] = CLK_DEFN(sclkepll_data),
    [CLK_SCLKVPLL] = CLK_DEFN(sclkvpll_data),
    [CLK_SPI0    ] = CLK_DEFN(spi0_data),
    [CLK_SPI1    ] = CLK_DEFN(spi1_data),
    [CLK_SPI2    ] = CLK_DEFN(spi2_data),
    [CLK_SPI0_ISP] = CLK_DEFN_NONE(),
    [CLK_SPI1_ISP] = CLK_DEFN_NONE(),
    [CLK_UART0   ] = CLK_DEFN(uart0_data),
    [CLK_UART1   ] = CLK_DEFN(uart1_data),
    [CLK_UART2   ] = CLK_DEFN(uart2_data),
    [CLK_UART3   ] = CLK_DEFN(uart3_data),
    [CLK_PWM     ] = CLK_DEFN_NONE(),
    [CLK_I2C0    ] = CLK_DEFN(i2c0_data),
    [CLK_I2C1    ] = CLK_DEFN(i2c1_data),
    [CLK_I2C2    ] = CLK_DEFN(i2c2_data),
    [CLK_I2C3    ] = CLK_DEFN(i2c3_data),
    [CLK_I2C4    ] = CLK_DEFN(i2c4_data),
    [CLK_I2C5    ] = CLK_DEFN(i2c5_data),
    [CLK_I2C6    ] = CLK_DEFN(i2c6_data),
    [CLK_I2C7    ] = CLK_DEFN(i2c7_data),
    [CLK_MMC0    ] = CLK_DEFN(mmc0_data),
    [CLK_MMC1    ] = CLK_DEFN(mmc1_data),
    [CLK_MMC2    ] = CLK_DEFN(mmc2_data),
    [CLK_MMC3    ] = CLK_DEFN(mmc3_data)
};

const struct device dev_cmu_cpu = {
    .name = "CMU_CPU",
    .pstart = CMU_CPU_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_cmu_core = {
    .name = "CMU_CORE",
    .pstart = CMU_CORE_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_cmu_acp = {
    .name = "CMU_ACP",
    .pstart = CMU_ACP_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_cmu_isp = {
    .name = "CMU_ISP",
    .pstart = CMU_ISP_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_cmu_top = {
    .name = "CMU_TOP",
    .pstart = CMU_TOP_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_cmu_cdrex = {
    .name = "CMU_CDREX",
    .pstart = CMU_CDREX_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

const struct device dev_cmu_mem = {
    .name = "CMU_MEM",
    .pstart = CMU_MEM_PADDR,
    .size = 0x1000,
    .handle_device_fault = NULL,
    .priv = NULL
};

static const struct device *clock_devices[] = {
    [CLKREGS_CPU  ] = &dev_cmu_cpu,
    [CLKREGS_CORE ] = &dev_cmu_core,
    [CLKREGS_ACP  ] = &dev_cmu_acp,
    [CLKREGS_ISP  ] = &dev_cmu_isp,
    [CLKREGS_TOP  ] = &dev_cmu_top,
    [CLKREGS_LEX  ] = NULL,
    [CLKREGS_R0X  ] = NULL,
    [CLKREGS_R1X  ] = NULL,
    [CLKREGS_CDREX] = &dev_cmu_cdrex,
    [CLKREGS_MEM  ] = &dev_cmu_mem,
};

struct clock_device {
    void *mask[NCLKREGS];
};


struct clock_device *
vm_install_ac_clock(vm_t *vm, enum vacdev_default default_ac, enum vacdev_action action)
{
    struct clock_device *clkd;
    int i;
    uint8_t ac = (default_ac == VACDEV_DEFAULT_ALLOW) ? 0xff : 0x00;
    /* Initialise private data */
    clkd = (struct clock_device *)calloc(1, sizeof(*clkd));
    if (clkd == NULL) {
        return NULL;
    }
    for (i = 0; i < NCLKREGS; i++) {
        int err;
        if (clock_devices[i] == NULL) {
            continue;
        }

        /* map device masks */
        clkd->mask[i] = calloc(1, BIT(12));
        if (clkd->mask[i] == NULL) {
            return NULL;
        }
        memset(clkd->mask[i], ac, BIT(12));
        /* Install generic access control */
        err = vm_install_generic_ac_device(vm, clock_devices[i], clkd->mask[i],
                                           BIT(12), action);
        if (err) {
            return NULL;
        }
    }
    return clkd;
}

int vm_clock_config(struct clock_device *clkd, enum clk_id clk_id, int grant)
{
    struct clock_data *data = &clock_data[clk_id];
    int i;
    assert(data);
    assert(data->nregs);
    for (i = 0; i < data->nregs; i++) {
        uint32_t *mask;
        uint32_t mask_val;
        mask = (uint32_t *)(clkd->mask[data->regs[i].bank] + data->regs[i].offset);
        /* MASK(x) does not handle 32 bit masks */
        mask_val = ((1UL << data->regs[i].bits) - 1) << data->regs[i].shift;
        if (grant) {
            *mask |= mask_val;
        } else {
            *mask &= ~mask_val;
        }
    }
    return 0;
}

int vm_clock_provide(struct clock_device *clkd, enum clk_id clk_id)
{
    return vm_clock_config(clkd, clk_id, 1);
}

int vm_clock_restrict(struct clock_device *clkd, enum clk_id clk_id)
{
    return vm_clock_config(clkd, clk_id, 0);
}
