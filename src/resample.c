#include "resample.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


resampler_t *resampler_create(const int ifactor, const int ofactor, buffer_t *in, buffer_t *out)
{
	if (in==NULL) {
		fprintf(stderr, "resampler_create: in==NULL\n");
		return NULL;
	}
	if (out==NULL) {
		fprintf(stderr, "resampler_create: out==NULL\n");
		return NULL;
	}
	if (ifactor==0) {
		fprintf(stderr, "resampler_create: ifactor=0\n");
		return NULL;
	}
	if (ofactor==0) {
		fprintf(stderr, "resampler_create: ofactor=0\n");
		return NULL;
	}
	resampler_t *r=malloc(sizeof(resampler_t));
	if (r==NULL) {
		fprintf(stderr, "resampler_create: malloc failed\n");
		return NULL;
	}
	memset(r, 0, sizeof(resampler_t));
	r->ifactor=ifactor;
	r->ofactor=ofactor;
	r->ibuffer=in;
	r->obuffer=out;
	r->last_sample=0;
	return r;
}

//Trivial resampling
int resampler_resample(resampler_t *r)
{
	if (r==NULL) return 0;

	int isamples=buffer_data(r->ibuffer)/r->ifactor;
	int osamples=buffer_space(r->obuffer)/r->ofactor;
	int min=(isamples<osamples)?isamples:osamples;

	for (int n=0; n<min; n++) {
		double sum=0;
		for (int m=0; m<r->ifactor; m++) {
			double v=0;
			int res=buffer_read(r->ibuffer, &v);
			if (res==0) {
				fprintf(stderr, "resampler_resample: %p buffer_read failed on buffer %p, sample %d %d min: %d\n", r, r->ibuffer, n, m, min);
				return 0;
			}
			sum=sum+v;
		}
		sum=sum/r->ifactor;
		for (int m=0; m<r->ofactor; m++) {
			double p=(m*1.0)/(r->ofactor*1.0); //trivial linear interpolation
			double o=(1-p)*r->last_sample + p*sum;
			int res=buffer_write(r->obuffer, o);
			if (res==0) {
				fprintf(stderr, "resampler_resample %p, buffer_write to %p failed\n", r, r->obuffer);
				return 0;
			}
		}
		r->last_sample=sum;
	}
	return 1;
}
