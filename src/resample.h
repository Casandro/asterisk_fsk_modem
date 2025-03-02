#pragma once

#include "buffer.h"

#include "filters.h"


typedef struct {
	int ifactor;
	int ofactor;
	double last_sample;
	filter_lp_t *filter;
	buffer_t *ibuffer;
	buffer_t *obuffer;
} resampler_t;


resampler_t *resampler_create(buffer_t *in, buffer_t *out);
int resampler_resample(resampler_t *r);
