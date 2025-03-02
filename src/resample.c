#include "resample.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/param.h>


int euclidean_gcd(int a, int b)
{
	if (a==b) return 1;
	if (b==0) return a;
	if (a==0) return b;
	if (b>a) return euclidean_gcd(b,a);
	return euclidean_gcd(b, a%b);
}


resampler_t *resampler_create(buffer_t *in, buffer_t *out)
{
	if (in==NULL) {
		fprintf(stderr, "resampler_create: in==NULL\n");
		return NULL;
	}
	if (out==NULL) {
		fprintf(stderr, "resampler_create: out==NULL\n");
		return NULL;
	}
	resampler_t *r=malloc(sizeof(resampler_t));
	if (r==NULL) {
		fprintf(stderr, "resampler_create: malloc failed\n");
		return NULL;
	}
	memset(r, 0, sizeof(resampler_t));
	int srate_i=in->srate;
	int srate_o=out->srate;
	int gcd=euclidean_gcd(srate_i, srate_o);
	r->ifactor=srate_i/gcd;
	r->ofactor=srate_o/gcd;
	r->ibuffer=in;
	r->obuffer=out;
	r->last_sample=0;


	if ( ! ((r->ifactor==1) || (r->ofactor==1))) {
		fprintf(stderr, "Resampling rate not a integer multiple\n");
		return NULL;
	}


	return r;
}

//Trivial resampling
int resampler_resample(resampler_t *r)
{
	if (r==NULL) return 0;

	int isamples=buffer_data(r->ibuffer)/r->ifactor;
	int osamples=buffer_space(r->obuffer)/r->ofactor;

	for (int n=0; n<MIN(isamples, osamples); n++) {
		double sum=0;
		for (int m=0; m<r->ifactor; m++) {
			double v=0;
			int res=buffer_read(r->ibuffer, &v);
			if (res==0) {
				fprintf(stderr, "resampler_resample: %p buffer_read failed on buffer %p, sample %d %d \n", r, r->ibuffer, n, m);
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
