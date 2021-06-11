/*
 * Copyright 2019, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#pragma once

#include <pb.h>
#include <sel4/sel4.h>

/* bind a nanopb stream to the IPC buffer of the thread */
pb_ostream_t pb_ostream_from_IPC(seL4_Word offset);
pb_istream_t pb_istream_from_IPC(seL4_Word offset);
