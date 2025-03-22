#pragma once

#include "buffer.h"
#include "dbuffer.h"

typedef struct {
	dbuffer_t *in;
	buffer_t *out;
} async_fsk_modulator_t;
