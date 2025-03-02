#include "filters.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>


double six(const double x)
{
	if (x==0) return 1;
	return sin(x)/x;
}


filter_lp_t *filter_lp_create(const double bandwidth, const double srate, const int order)
{
	filter_lp_t *lp=calloc(sizeof(filter_lp_t), 1);
	if (lp==NULL) {
		fprintf(stderr, "filter_lp_create, main calloc failed\n");
		return NULL;
	}
	lp->order=order;
	lp->buffer=calloc(sizeof(double), lp->order);
	if (lp->buffer==NULL) {
		fprintf(stderr, "filter_lp_create, 2nd calloc failed\n");
		free(lp);
		return NULL;
	}

	lp->sinx_buffer=calloc(sizeof(double), lp->order);
	if (lp->sinx_buffer==NULL) {
		fprintf(stderr, "filter_lp_create, 3rd calloc failed\n");
		free(lp->buffer);
		free(lp);
		return NULL;
	}
	for (int n=0; n<lp->order; n++) {
		double phi=(n-order/2)*(bandwidth/srate)*M_PI*2;
		lp->sinx_buffer[n]=six(phi);
	}
	return lp;
}

void filter_lp_free(filter_lp_t *lp)
{
	if (lp==NULL) return;
	if (lp->buffer!=NULL) free(lp->buffer);
	if (lp->sinx_buffer!=NULL) free(lp->sinx_buffer);
	free(lp);
}

void filter_lp_put(filter_lp_t *lp, const double input)
{
	if (lp==NULL) return;
	lp->pointer=(lp->pointer+1)%lp->order;
	lp->buffer[lp->pointer]=input;
}

double filter_lp_get(filter_lp_t *lp)
{
	if (lp==NULL) return 0;
	double sum=0;
	for (int n=0; n<lp->order; n++) {
		sum=sum+lp->sinx_buffer[n]*lp->buffer[(lp->order-n+lp->pointer)%lp->order];
	}
	return sum;
}

double filter_lp_filter(filter_lp_t *lp, const double input)
{
	if (lp==NULL) return 0;
	filter_lp_put(lp, input);
	return filter_lp_get(lp);
}


