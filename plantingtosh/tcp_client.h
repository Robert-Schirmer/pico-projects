/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _inc_tcp_client
#define _inc_tcp_client

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define TCP_SERVER_IP "192.168.1.220"
#define TCP_PORT 4242
#define DEBUG_printf printf
#define BUF_SIZE 2048

#define POLL_TIME_S 5

typedef struct TCP_CLIENT_T_
{
    struct tcp_pcb *tcp_pcb;
    ip_addr_t remote_addr;
    uint8_t buffer[BUF_SIZE];
    int buffer_len;
    bool complete;
    bool connected;
    int status;
    char *initial_msg;
} TCP_CLIENT_T;

typedef struct TCP_SERVER_RESPONSE_T_
{
    bool success;
    char *data;
    int data_len;
} TCP_SERVER_RESPONSE_T;

void free_response(TCP_SERVER_RESPONSE_T *res);

TCP_SERVER_RESPONSE_T *send_to_server(char *msg, void (*work_while_polling)(void *arg, uint poll_count), void *arg);

#endif
