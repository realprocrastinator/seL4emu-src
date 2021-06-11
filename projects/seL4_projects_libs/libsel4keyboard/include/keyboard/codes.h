/*
 * Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

/* Convert a keyboard scan code (from set 2) to its character representation.
 * Returns 0 for anything that is not a scan code for a canonical character.
 */
char sel4keyboard_code_to_char(int index);
