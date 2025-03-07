#pragma once

typedef struct {
	int order;
	int pointer;
	double *buffer;
	double *sinx_buffer;
} filter_lp_t;


filter_lp_t *filter_lp_create(const double bandwidth, const double srate, const int order);
void filter_lp_free(filter_lp_t *lp);
double filter_lp_filter(filter_lp_t *lp, const double input);

