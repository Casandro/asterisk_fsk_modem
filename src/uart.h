#pragma once

#include "dbuffer.h"
#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>

#define UART_BREAK (-2)
#define UART_PAUSE (-3)


typedef struct {
	int brate;
	dbuffer_t *in;
	dbuffer_t *out;
} uart_tx_t;



uart_tx_t *uart_tx_create(const int brate, dbuffer_t *data, dbuffer_t *bits);
void uart_tx_free(uart_tx_t *tx);

void uart_tx(uart_tx_t *tx);


typedef struct {
	int brate;
	int state;
	int bitlen;
	int word;
	buffer_t *in;
	dbuffer_t *out;
} uart_rx_t;

uart_rx_t *uart_rx_create(const int brate);
