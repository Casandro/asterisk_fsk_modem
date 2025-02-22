#pragma once

#include "buffer.h"

#define DEMOD_FSK_FILTER_ORDER (256)

typedef struct {
	buffer_t *in;
	buffer_t *out;
	double frq_0;
	double frq_1;
	double bandwidth;
	double phi;
	int delay_pointer;
	int srate;
	double power_avg;
	int state;
	double i_[3];
	double q_[3];
	double delays_i[DEMOD_FSK_FILTER_ORDER];
	double delays_q[DEMOD_FSK_FILTER_ORDER];
} demodulator_fsk_t;
