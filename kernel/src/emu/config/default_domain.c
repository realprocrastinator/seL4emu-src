/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include <model/statedata.h>
#include <object/structures.h>

/* Default schedule. */
const dschedule_t ksDomSchedule[] = {
    {.domain = 0, .length = 1},
};

const word_t ksDomScheduleLength = sizeof(ksDomSchedule) / sizeof(dschedule_t);
