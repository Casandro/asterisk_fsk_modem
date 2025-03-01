#pragma once

#include "buffer.h"


typedef struct {
	int ifactor;
	int ofactor;
	double last_sample;
	buffer_t *ibuffer;
	buffer_t *obuffer;
} resampler_t;


resampler_t *resampler_create(const int ifactor, const int ofactor, buffer_t *in, buffer_t *out);
int resampler_resample(resampler_t *r);
