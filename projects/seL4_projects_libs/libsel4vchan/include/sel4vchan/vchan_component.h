/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#pragma once

#include "libvchan.h"
#include "vchan_sharemem.h"
#include "vchan_copy.h"

typedef void (*callback_func_t)(void *);

typedef struct camkes_vchan_con {
    /* Domain */
    int dest_dom_number;
    int source_dom_number;
    void *data_buf;

    /* Function Pointers */
    int (*connect)(vchan_connect_t);
    int (*disconnect)(vchan_connect_t);
    intptr_t (*get_buf)(vchan_ctrl_t, int);

    int (*status)(vchan_ctrl_t);
    int (*alert_status)(vchan_ctrl_t, int *data_ready, int *buffer_space);

    void (*wait)(void);
    void (*alert)(void);
    int (*poll)(void);

    int (*reg_callback)(callback_func_t, void *);
} camkes_vchan_con_t;


struct libvchan {
    int is_server;
    int server_persists;
    int blocking;
    int domain_num, port_num;

    camkes_vchan_con_t *con;
};

libvchan_t *link_vchan_comp(libvchan_t *ctrl, camkes_vchan_con_t *vchan_com);
vchan_buf_t *get_vchan_buf(vchan_ctrl_t *args, camkes_vchan_con_t *c, int action);
