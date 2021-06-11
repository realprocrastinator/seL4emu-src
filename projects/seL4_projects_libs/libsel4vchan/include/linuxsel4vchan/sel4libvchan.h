/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#pragma once


#define DEF_READ_SIZE  1024
#define DEF_WRITE_SIZE 2048

struct libvchan {
    struct libsel4vchan *sel4vchan;
};

typedef struct libsel4vchan {
    int is_server;
    int server_persists;
    int blocking;
    int domain_num, port_num;
    int driver_fd, event_fd;
} libsel4vchan_t;

struct libsel4vchan *libsel4vchan_server_init(int domain, int port);
struct libsel4vchan *libsel4vchan_client_init(int domain, int port);

void libsel4vchan_close(struct libsel4vchan *vchan);
int libsel4vchan_is_open(struct libsel4vchan *vchan);
int libsel4vchan_data_ready(struct libsel4vchan *vchan);
int libsel4vchan_buffer_space(struct libsel4vchan *vchan);
