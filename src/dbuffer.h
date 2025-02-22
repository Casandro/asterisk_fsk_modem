#pragma once

#include <stdint.h>

#define DBUFFER_LEN (128)

typedef struct {
	int read_pointer;
	int write_pointer;
	int16_t buffer[DBUFFER_LEN];
} dbuffer_t;
