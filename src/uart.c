#include "uart.h"
#include "dbuffer.h"

uart_tx_t *uart_tx_create(const int brate, dbuffer_t *data, dbuffer_t *bits)
{
	uart_tx_t *tx=calloc(sizeof(uart_tx_t),1);
	if (tx==NULL) {
		fprintf(stderr, "uart_tx create: calloc failed\n");
		return NULL;
	}
}

void uart_tx_free(uart_tx_t *tx)
{
	free(tx);
}

void uart_tx(uart_tx_t *tx)
{
	while (dbuffer_space(tx->out)>15) {
		int d=dbuffer_get(tx->out);
		if (d==DBUFFER_IDLE) { //No data in buffer => don't insert any data, modulator will insert mark (1) tone
			return;
		}
		if (d==UART_BREAK) {
			for (int n=0; n<14; n++) {
				dbuffer_put(tx->out, 0);
			}
			dbuffer_put(tx->out, 1); //Stop bit
			continue;
		}	
		if (d==UART_PAUSE) {
			for (int n=0; n<15; n++) {
				dbuffer_put(tx->out, 1); //Idle bits
			}
			continue;
		}
		if ( (d>=0) && (d<=0xff) ) {
			dbuffer_put(tx->out, 0); //Start bit
			for (int n=0; n<8; n++) {
				dbuffer_put(tx->out, (d>>n)&0x1);
			}
			dbuffer_put(tx->out, 1); //Stop bit
			continue;
		}
		// We should never get here, let's place an idle bit here
		dbuffer_put(tx->out, 1); //idle bit
	}
}

uart_rx_t *uart_rx_create(const int brate)
{
	uart_rx_t *tx=calloc(sizeof(uart_rx_t),1);
	if (tx==NULL) {
		return NULL;
	}
	tx->brate=brate;
	return tx;
}
