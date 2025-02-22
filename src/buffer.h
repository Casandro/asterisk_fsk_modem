#pragma once

typedef struct {
	int size;
	int read_pointer;
	int write_pointer;
	double *buffer;
} buffer_t;


buffer_t *buffer_create(const int length);
void buffer_free(buffer_t *b);

//Returns the amount of samples we can read from the buffer
int buffer_data(const buffer_t *b);

//Returns the amount of free space in our buffer
int buffer_space(const buffer_t *b);

//Write a value to the buffer
int buffer_write(buffer_t *b, double v);

//Read a value from the buffer
int buffer_read(buffer_t *b, double *v);


