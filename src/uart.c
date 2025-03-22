#include "uart.h"
#include "dbuffer.h"

uart_tx_t *uart_tx_create(const int brate, dbuffer_t *data, dbuffer_t *bits)
{
	uart_tx_t *tx=calloc(sizeof(uart_tx_t),1);
	if (tx==NULL) {
		fprintf(stderr, "uart_tx create: calloc failed\n");
		return NULL;
	}
	tx->in=data;
	tx->out=bits;
	tx->brate=brate;
	return tx;
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

uart_rx_t *uart_rx_create(buffer_t *in, dbuffer_t *out, const int brate)
{
	uart_rx_t *tx=calloc(sizeof(uart_rx_t),1);
	if (tx==NULL) {
		return NULL;
	}
	int in_rate=in->srate;
	tx->brate=brate;
	tx->bitlen=in_rate/brate;
	tx->state=-1;
	return tx;
}

void uart_rx(uart_rx_t *u)
{
	int len=buffer_data(u->in);
	for (int n=0; n<len; n++) {
		double v=0;
		int res=buffer_read(y->in, &v);
		if (res==0) {
			fprintf(stderr, "uart_rx: reading failed\n");
			return res;
		}
		int bit=(v<0)?0:1;
		if ( (u->state==-2) && (bit==1) ) { //End of break condition or error
			u->state=-1;
			if (u->word==0)	dbuffer(u->out, UART_BREAK); // Break
		}
		if ( (u->state==-1) && (bit==0) ) {
			u->state=0;
			continue;
		}
		//We are now inside a data period
		int bitnum=u->state/u->bitlen;
		int bitpos=u->state%u->bitlen;
		if (bitpos==u->bitlen/2) {
			if ( (bitnum==0) && (bit==1) ) { //incorrect start bit
				c->status=-1;
				continue;
			}
			if ( (bitnum>=1) && (bitnum<=8) ){ //Data bit
				c->word=(c->word<<1)| (bit<<7);
				continue;
			}
			if (bitnum==9) { //Stop bit
				if (bit==1) { //Normal stop bit
					dbuffer(u->out, c->word&0xff);
					u->word=0;
					u->status=-1;
					continue;
				}
				if (bit==0) { //No stop bit
					if (word==0x00) { //All Bits zero => break
						u->status=-2;
						continue;
					}
					u->status=-2; //Error state wait for 
				}
			}
		}
	}
}
