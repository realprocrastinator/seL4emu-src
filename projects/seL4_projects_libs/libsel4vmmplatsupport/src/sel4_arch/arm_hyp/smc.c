/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */


#include <sel4vm/guest_vm.h>

#include "smc.h"

seL4_Word smc_get_function_id(seL4_UserContext *u)
{
    return u->r0;
}

seL4_Word smc_set_return_value(seL4_UserContext *u, seL4_Word val)
{
    u->r0 = val;
}

seL4_Word smc_get_arg(seL4_UserContext *u, seL4_Word arg)
{
    switch (arg) {
    case 1:
        return u->r1;
    case 2:
        return u->r2;
    case 3:
        return u->r3;
    case 4:
        return u->r4;
    case 5:
        return u->r5;
    case 6:
        return u->r6;
    default:
        ZF_LOGF("SMC only has 6 argument registers");
    }
}

void smc_set_arg(seL4_UserContext *u, seL4_Word arg, seL4_Word val)
{
    switch (arg) {
    case 1:
        u->r1 = val;
        break;
    case 2:
        u->r2 = val;
        break;
    case 3:
        u->r3 = val;
        break;
    case 4:
        u->r4 = val;
        break;
    case 5:
        u->r5 = val;
        break;
    case 6:
        u->r6 = val;
        break;
    default:
        ZF_LOGF("SMC only has 6 argument registers");
    }
}
