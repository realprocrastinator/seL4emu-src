/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */


#include <sel4vm/guest_vm.h>

#include "smc.h"

seL4_Word smc_get_function_id(seL4_UserContext *u)
{
    return u->x0;
}

seL4_Word smc_set_return_value(seL4_UserContext *u, seL4_Word val)
{
    u->x0 = val;
}

seL4_Word smc_get_arg(seL4_UserContext *u, seL4_Word arg)
{
    switch (arg) {
    case 1:
        return u->x1;
    case 2:
        return u->x2;
    case 3:
        return u->x3;
    case 4:
        return u->x4;
    case 5:
        return u->x5;
    case 6:
        return u->x6;
    default:
        ZF_LOGF("SMC only has 6 argument registers");
    }
}

void smc_set_arg(seL4_UserContext *u, seL4_Word arg, seL4_Word val)
{
    switch (arg) {
    case 1:
        u->x1 = val;
        break;
    case 2:
        u->x2 = val;
        break;
    case 3:
        u->x3 = val;
        break;
    case 4:
        u->x4 = val;
        break;
    case 5:
        u->x5 = val;
        break;
    case 6:
        u->x6 = val;
        break;
    default:
        ZF_LOGF("SMC only has 6 argument registers");
    }
}
