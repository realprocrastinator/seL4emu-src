/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#pragma once

#include "libvchan.h"

#define VCHAN_EVENT_IRQ 92

#define DATATYPE_INT        0
#define DATATYPE_STRING     1

/* Defines for copying/reading data across vchans */
#define VCHAN_PACKET_SIZE 4096
#define FILE_DATAPORT_MAX_SIZE 4096

#define NOWAIT_DATA_READY 1
#define NOWAIT_BUF_SPACE 2

#define VCHAN_STREAM    1
#define VCHAN_NOSTREAM  0

/* Used in arguments referring to a vchan instance */
typedef struct vchan_ctrl {
    int domain;
    int dest;
    int port;
} vchan_ctrl_t;

/* Used in arguments referring to a vchan instance */
typedef struct vchan_alert {
    int buffer_space;
    int data_ready;
    int dest;
    int port;
} vchan_alert_t;

/* Arguments structure used for vchan write/read actions between guest os's */
typedef struct vchan_args {
    vchan_ctrl_t v;
    void *mmap_ptr;
    int stream;
    int size;
    int mmap_phys_ptr;
} vchan_args_t;

/* Argument structure used for vchan  */
typedef struct vchan_check_args {
    vchan_ctrl_t v;
    int nowait;
    int state;
    int checktype;
} vchan_check_args_t;

/* Argument structure used for vchan connects  */
typedef struct vchan_connect {
    vchan_ctrl_t v;
    int server;
    int eventfd;
    unsigned event_mon;
} vchan_connect_t;

int libvchan_readwrite(libvchan_t *ctrl, void *data, size_t size, int cmd, int stream);
