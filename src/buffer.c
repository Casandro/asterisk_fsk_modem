#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

buffer_t *buffer_create(const int length, const int srate)
{
	buffer_t *b=malloc(sizeof(buffer_t));
	if (b==NULL) {
		fprintf(stderr, "create_buffer: malloc failed for buffer_t\n");
		return NULL;
	}
	memset(b, 0, sizeof(buffer_t));
	b->buffer=malloc(sizeof(double)*length);
	if (b->buffer==NULL) {
		fprintf(stderr, "create_buffer: malloc failed for %d elements\n", length);
		free(b);
		return NULL;
	}
	b->size=length;
	b->read_pointer=0;
	b->write_pointer=0;
	b->srate=srate;
	return b;
}

void buffer_free(buffer_t *b)
{
	if (b==NULL) return;
	if (b->buffer!=NULL) free(b->buffer);
	free(b);
}

//Returns the amount of samples we can read from the buffer
int buffer_data(const buffer_t *b)
{
	if (b==NULL) return 0;
	if (b->write_pointer==b->read_pointer) return 0; //Read and write pointer at same position => no data
	if (b->write_pointer>b->read_pointer) return b->write_pointer-b->read_pointer;
	if (b->write_pointer<b->read_pointer) return b->size+b->write_pointer-b->read_pointer;
	return 0; //We should never get here
}

//Returns the amount of free space in our buffer
int buffer_space(const buffer_t *b)
{
	if (b==NULL) return 0;
	return b->size-buffer_data(b)-1;
}

int buffer_write(buffer_t *b, double v)
{
	if (b==NULL) return 0;
	int np=(b->write_pointer+1)%b->size;
	if (np==b->read_pointer) {
		fprintf(stderr, "buffer.h buffer_write: buffer overrun %p\n", b);
		return 0;
	}
	b->buffer[b->write_pointer]=v;
	b->write_pointer=np;
	return 1;
}


int buffer_read(buffer_t *b, double *v)
{
	if (b==NULL) return 0;
	if (v==NULL) return 0;
	if (b->read_pointer==b->write_pointer) {
		fprintf(stderr, "buffer.h buffer_read: buffer underrun %p\n", b);
		return 0;
	}
	int np=(b->read_pointer+1)%b->size;
	*v=b->buffer[b->read_pointer];
	b->read_pointer=np;
	return 1;
}
