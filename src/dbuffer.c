#include "dbuffer.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

dbuffer_t *dbuffer_create(const int len)
{
	dbuffer_t *b=calloc(sizeof(dbuffer_t),1);
	if (b==NULL) {
		fprintf(stderr, "dbuffer_create: calloc failed\n");
		return NULL;
	}
	b->len=len;
	b->buffer=calloc(sizeof(int16_t), len);
	if (b->buffer==NULL) {
		fprintf(stderr, "dbuffer_create: calloc failed 2\n");
		free(b);
		return NULL;
	}
	return b;
}

void dbuffer_free(dbuffer_t *b)
{
	if (b==NULL) return;
	if (b->buffer!=NULL) free(b->buffer);
	free(b);
}


int dbuffer_put(dbuffer_t *b, int16_t w)
{
	int nwp=(b->wp+1)%b->len;
	if (nwp==b->rp) {
		fprintf(stderr, "dbuffer_put: Buffer overrun\n");
		return 0;
	}
	b->buffer[nwp]=w;
	b->wp=nwp;
	return 1;
}

int16_t dbuffer_get(dbuffer_t *b)
{
	if (b->wp==b->rp) {
		return DBUFFER_IDLE;
	}
	int nrp=(b->rp+1)%b->len;
	int16_t o=b->buffer[b->rp];
	b->rp=nrp;
	return o;
}

int dbuffer_data(const dbuffer_t *b)
{
	if (b==NULL) return 0;
	if (b->wp==b->rp) return 0; //Read and write pointer at same position => no data
	if (b->wp>b->rp) return b->wp-b->rp;
	if (b->wp<b->rp) return b->len+b->wp-b->rp;
	return 0; //We should never get here
}

//Returns the amount of free space in our buffer
int dbuffer_space(const dbuffer_t *b)
{
	if (b==NULL) return 0;
	return b->len-dbuffer_data(b)-1;
}

