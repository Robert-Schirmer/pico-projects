/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "tcp_client.h"

#if 0
static void dump_bytes(const uint8_t *bptr, uint32_t len)
{
    unsigned int i = 0;

    printf("dump_bytes %d", len);
    for (i = 0; i < len;)
    {
        if ((i & 0x0f) == 0)
        {
            printf("\n");
        }
        else if ((i & 0x07) == 0)
        {
            printf(" ");
        }
        printf("%02x ", bptr[i++]);
    }
    printf("\n");
}
#define DUMP_BYTES dump_bytes
#else
#define DUMP_BYTES(A, B)
#endif

#if 1
#define DEBUG_printf printf
#else
static void empty_printf(const char *format, ...)
{
}
#define DEBUG_printf empty_printf
#endif

static TCP_SERVER_RESPONSE_T *response_result(TCP_CLIENT_T *state)
{
    TCP_SERVER_RESPONSE_T *response = calloc(1, sizeof(TCP_SERVER_RESPONSE_T));

    if (state->status == 0)
    {
        DEBUG_printf("response_result success\n");
        response->success = true;
        DEBUG_printf("response_result copying used bytes in buffer to response: %d\n", state->buffer_len);
        DEBUG_printf("response_result buffer: %s\n", state->buffer);
        response->data_len = state->buffer_len;
        response->data = malloc(state->buffer_len + 1);
        memcpy(response->data, state->buffer, state->buffer_len);
        response->data[state->buffer_len] = '\0';

        return response;
    }

    DEBUG_printf("response_result failed\n");
    response->success = false;
    char *error_msg = "Response error (";
    char error_code[2];
    sprintf(error_code, "%d", state->status);
    char *error_msg_close = ")";
    response->data = malloc(strlen(error_msg) + strlen(error_code) + strlen(error_msg_close) + 1);
    strcpy(response->data, error_msg);
    strcat(response->data, error_code);
    strcat(response->data, error_msg_close);
    return response;
}

static err_t tcp_client_close(void *arg)
{
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    err_t err = ERR_OK;
    if (state->tcp_pcb != NULL)
    {
        tcp_arg(state->tcp_pcb, NULL);
        tcp_poll(state->tcp_pcb, NULL, 0);
        tcp_sent(state->tcp_pcb, NULL);
        tcp_recv(state->tcp_pcb, NULL);
        tcp_err(state->tcp_pcb, NULL);
        err = tcp_close(state->tcp_pcb);
        if (err != ERR_OK)
        {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(state->tcp_pcb);
            err = ERR_ABRT;
        }
        state->tcp_pcb = NULL;
    }

    free(state->initial_msg);

    state->complete_callback(response_result(state));

    free(state);

    return err;
}

// Called with results of operation
static err_t tcp_result(void *arg, int status)
{
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    if (status == 0)
    {
        DEBUG_printf("tcp request success\n");
    }
    else
    {
        DEBUG_printf("tcp request failed %d\n", status);
    }
    state->complete = true;
    state->status = status;

    return tcp_client_close(arg);
}

static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    DEBUG_printf("tcp_client_sent %u\n", len);

    return ERR_OK;
}

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    if (err != ERR_OK)
    {
        DEBUG_printf("tcp_client_connected connect failed %d\n", err);
        return tcp_result(arg, err);
    }
    state->connected = true;
    DEBUG_printf("tcp_client_connected\n");
    if (state->initial_msg)
    {
        err_t err = tcp_write(state->tcp_pcb, state->initial_msg, strlen(state->initial_msg), TCP_WRITE_FLAG_COPY);
        if (err != ERR_OK)
        {
            DEBUG_printf("send_to_server: write failed %d\n", err);
            return tcp_result(state, err);
        }
    }

    return ERR_OK;
}

static err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
    // Idle connection, close it
    DEBUG_printf("tcp_client_poll\n");
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;

    return tcp_result(arg, ERR_CLSD);
}

static void tcp_client_err(void *arg, err_t err)
{
    if (err != ERR_ABRT)
    {
        DEBUG_printf("tcp_client_err %d\n", err);
        tcp_result(arg, err);
    }
    else
    {
        DEBUG_printf("tcp_client_err %d\n", err);
    }
}

err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    if (!p)
    {
        return tcp_result(arg, -1);
    }
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    if (p->tot_len > 0)
    {
        DEBUG_printf("recv %d err %d\n", p->tot_len, err);
        for (struct pbuf *q = p; q != NULL; q = q->next)
        {
            DUMP_BYTES(q->payload, q->len);
        }
        // Receive the buffer
        const uint16_t buffer_left = BUF_SIZE - state->buffer_len;
        if (buffer_left == 0)
        {
            DEBUG_printf("recv buffer full\n");
            return tcp_result(arg, -1);
        }
        state->buffer_len += pbuf_copy_partial(p, state->buffer + state->buffer_len,
                                               p->tot_len > buffer_left ? buffer_left : p->tot_len, 0);
        tcp_recved(tpcb, p->tot_len);
    }
    pbuf_free(p);

    char *end_cahr = &state->buffer[state->buffer_len - 3];
    if (strcmp("END", end_cahr) == 0)
    {
        state->buffer_len -= 3;
        DEBUG_printf("recv END\n");
        return tcp_result(arg, ERR_OK);
    }

    return ERR_OK;
}

static bool tcp_client_open(void *arg)
{
    TCP_CLIENT_T *state = (TCP_CLIENT_T *)arg;
    DEBUG_printf("tcp_client_open connecting to %s port %u\n", ip4addr_ntoa(&state->remote_addr), TCP_PORT);
    state->tcp_pcb = tcp_new_ip_type(IP_GET_TYPE(&state->remote_addr));
    if (!state->tcp_pcb)
    {
        DEBUG_printf("tcp_client_open failed to create pcb\n");
        return false;
    }

    tcp_arg(state->tcp_pcb, state);
    tcp_poll(state->tcp_pcb, tcp_client_poll, POLL_TIME_S * 2);
    tcp_sent(state->tcp_pcb, tcp_client_sent);
    tcp_recv(state->tcp_pcb, tcp_client_recv);
    tcp_err(state->tcp_pcb, tcp_client_err);

    state->buffer_len = 0;

    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();
    err_t err = tcp_connect(state->tcp_pcb, &state->remote_addr, TCP_PORT, tcp_client_connected);
    cyw43_arch_lwip_end();

    return err == ERR_OK;
}

// Perform initialisation
static TCP_CLIENT_T *tcp_client_init(void)
{
    TCP_CLIENT_T *state = calloc(1, sizeof(TCP_CLIENT_T));
    if (!state)
    {
        DEBUG_printf("tcp_client_init failed to allocate state\n");
        return NULL;
    }
    ip4addr_aton(TCP_SERVER_IP, &state->remote_addr);
    return state;
}

void free_response(TCP_SERVER_RESPONSE_T *res)
{
    if (res)
    {
        if (res->data)
        {
            DEBUG_printf("free_response freeing response data\n");
            free(res->data);
        }
        DEBUG_printf("free_response freeing response\n");
        free(res);
    }
}

void send_to_server(char *data, void (*complete_callback)(TCP_SERVER_RESPONSE_T *res))
{
    TCP_CLIENT_T *state = tcp_client_init();
    if (!state)
    {
        complete_callback(response_result(state));
    }

    state->initial_msg = malloc(strlen(data) + 1);
    strcpy(state->initial_msg, data);
    state->complete_callback = complete_callback;

    if (!tcp_client_open(state))
    {
        tcp_result(state, ERR_RST);
    }
}
