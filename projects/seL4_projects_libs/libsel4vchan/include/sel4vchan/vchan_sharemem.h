/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sel4/sel4.h>
#include <sel4utils/util.h>
#include <simple/simple.h>


#define VCHAN_BUF_SIZE PAGE_SIZE_4K
#define NUM_SHARED_VCHAN_BUFFERS 2

typedef struct vchan_buf {
    int owner;
    int read_pos, write_pos;
    char sync_data[VCHAN_BUF_SIZE];
} vchan_buf_t;

/*
    Handles managing of packets, storing packets in shared mem,
        copying in memory and reading from memory for sync comms
*/
typedef struct vchan_shared_mem {
    int alloced;
    vchan_buf_t bufs[2];
} vchan_shared_mem_t;

typedef struct vchan_shared_mem_headers {
    vchan_shared_mem_t shared_buffers[NUM_SHARED_VCHAN_BUFFERS];
    int token;
} vchan_headers_t;
