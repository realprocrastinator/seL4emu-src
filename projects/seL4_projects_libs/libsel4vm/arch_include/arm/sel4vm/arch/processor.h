/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#pragma once

#define HSR_MAX_EXCEPTION           (0x3f)

#define HSR_EXCEPTION_CLASS_SHIFT   (26)
#define HSR_EXCEPTION_CLASS_MASK    (HSR_MAX_EXCEPTION << HSR_EXCEPTION_CLASS_SHIFT)
#define HSR_EXCEPTION_CLASS(hsr)    (((hsr) & HSR_EXCEPTION_CLASS_MASK) >> HSR_EXCEPTION_CLASS_SHIFT)

#define HSR_IL_SHIFT                (25)
#define HSR_IL_MASK                 (1 << HSR_IL_SHIFT)
#define HSR_IL(hsr)                 (((hsr) & HSR_IL_MASK) >> HSR_IL_SHIFT)

#define HSR_ISS_MASK                (HSR_IL_MASK - 1)
