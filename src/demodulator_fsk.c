#include "demodulator_fsk.h"
#include "buffer.h"
#include "filters.h"
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>


demodulator_fsk_t *demodulator_fsk_create(buffer_t *in, buffer_t *out, const double frq_0, const double frq_1, const double bitrate)
{
	if ( (in==NULL) || (out==NULL) ) {
		syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_ERR), "demodulator_fsk_create in %p or out %p buffer is NULL", in, out);
		return NULL;
	}
	demodulator_fsk_t *dem=calloc(sizeof(demodulator_fsk_t), 1);
	if (dem==NULL) {
		syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_ERR), "demodulator_fsk_create: maloc failed");
		return NULL;
	}
	dem->in=in;
	dem->out=out;
	dem->frq_0=frq_0;
	dem->frq_1=frq_1;
	dem->f_factor=(frq_1-frq_0)*2*M_PI/(in->srate*1.0);
	dem->phi=0;
	double bw=(fabs(frq_0-frq_1)+bitrate/2);
	dem->filter_i=filter_lp_create(bw, in->srate, 64);
	dem->filter_q=filter_lp_create(bw, in->srate, 64);
	dem->srate=in->srate;
	dem->state=-1;
	dem->omega=(frq_0+frq_1)/2*2*M_PI/dem->srate;
	return dem;
}

void demodulator_fsk_free(demodulator_fsk_t *dem)
{
	filter_lp_free(dem->filter_i);
	filter_lp_free(dem->filter_q);
	free(dem);
}


int demodulator_fsk_demod(demodulator_fsk_t *dem)
{
	if (dem==NULL) {
		syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_ERR), "demodulator_fsk_demod parameter NULL");
		return 0;
	}
	int no_samples=buffer_data(dem->in);
	if (no_samples<=0) {
		syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_WARNING), "demodulator_fsk_demod no samples to demodulate");
		return 1; //Nothing to demodulate
	}
	if (buffer_space(dem->out)<no_samples) no_samples=buffer_space(dem->out);

	for (int n=0; n<no_samples; n++) {
		double isample=0;
		int res=buffer_read(dem->in, &isample);
		if (res<1) {
			syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_ERR), "demodulator_fsk_demod, nothing to read from buffer %p", dem->in);
			return res;
		}
		dem->phi=dem->phi+dem->omega;
		if (dem->phi>2*M_PI) dem->phi=dem->phi-(2*M_PI);

		dem->i_[1]=dem->i_[0];
		dem->q_[1]=dem->q_[0];
		dem->i_[0]=filter_lp_filter(dem->filter_i, cos(dem->phi)*isample);
		dem->q_[0]=filter_lp_filter(dem->filter_q, sin(dem->phi)*isample);


		double i= (dem->i_[1]+dem->i_[0])/2;
		double q= (dem->q_[1]+dem->q_[0])/2;

		double power=i*i+q*q;
		dem->power_avg=(dem->power_avg*0.9999)+power*0.0001;

		//The carrier is on, so we can demodulate
		//Determine derivatives
		double i_d=dem->i_[1]-dem->i_[0];
		double q_d=dem->q_[1]-dem->q_[0];
		double f_=q*i_d - i*q_d;
		double frq=-(f_/power)/dem->f_factor;
		res=buffer_write(dem->out, frq*0.01);
		if (res<1) {
			syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_ERR), "demodulator_fsk_demod, couldn't write to %p", dem->out);
			fprintf(stderr, "demodulator_fsk_demod, couldn't write to %p", dem->out);
			return res;
		}
	}
//	syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_DEBUG), "demodulator_fsk_demod, power %f", dem->power_avg);
	fprintf(stderr, "demodulator_fsk_demod, power %f %f %f\n", dem->power_avg, dem->f_factor, dem->f_factor);
	return 1;

}
