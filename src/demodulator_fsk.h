#pragma once

#include "buffer.h"
#include "filters.h"

#define DEMOD_FSK_FILTER_ORDER (200)

typedef struct {
	buffer_t *in;
	buffer_t *out;
	double frq_0;
	double frq_1;
	double f_factor;
	double omega;
	double phi;
	int delay_pointer;
	int srate;
	double power_avg;
	int state;
	double i_[2];
	double q_[2];
	filter_lp_t *filter_i;
	filter_lp_t *filter_q;
} demodulator_fsk_t;

demodulator_fsk_t *demodulator_fsk_create(buffer_t *in, buffer_t *out, const double frq_0, const double frq_1, const double bitrate);
void demodulator_fsk_free(demodulator_fsk_t *dem);
int demodulator_fsk_demod(demodulator_fsk_t *dem);
