#include "demodulator_fsk.h"
#include "buffer.h"
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>


demodulator_fsk_t *demodulator_fsk_create(const int srate, buffer_t *in, buffer_t *out, const double frq_0, const double frq_1, const double bandwidth)
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
	dem->phi=0;
	dem->bandwidth=bandwidth;
	dem->srate=srate;
	dem->state=-1;
	return dem;
}

double six(double x)
{
	if (x==0) return 1;
	return sin(x)/x;
}

double demodulator_fsk_filter(const double *buffer, const int p, const double bandwidth, const double srate)
{
	double sum=0;

	for (int n=0; n<DEMOD_FSK_FILTER_ORDER; n++) {
		double phi=(n-DEMOD_FSK_FILTER_ORDER/2)*(bandwidth/srate)/M_PI*2;
		sum=sum+six(phi)*buffer[(DEMOD_FSK_FILTER_ORDER-n+p)%DEMOD_FSK_FILTER_ORDER];
	}
	return sum;
}

double sqr(const double x)
{
	return x*x;
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

	double omega=(dem->frq_0+dem->frq_1)/2*2*M_PI/dem->srate;

	for (int n=0; n<no_samples; n++) {
		double isample=0;
		int res=buffer_read(dem->in, &isample);
		if (res<1) {
			syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_ERR), "demodulator_fsk_demod, nothing to read from buffer %p", dem->in);
			return res;
		}
		double i=cos(dem->phi)*isample;
		double q=sin(dem->phi)*isample;
		dem->phi=dem->phi+omega;
		if (dem->phi>2*M_PI) dem->phi=dem->phi-(2*M_PI);
		dem->delay_pointer=(dem->delay_pointer+1) % DEMOD_FSK_FILTER_ORDER;
		dem->delays_i[dem->delay_pointer]=i;
		dem->delays_q[dem->delay_pointer]=q;
		dem->i_[2]=dem->i_[1]; dem->i_[1]=dem->i_[0];
		dem->q_[2]=dem->q_[1]; dem->q_[1]=dem->q_[0];
		dem->i_[0]=demodulator_fsk_filter(dem->delays_i, dem->delay_pointer, dem->bandwidth, dem->srate);
		dem->q_[0]=demodulator_fsk_filter(dem->delays_q, dem->delay_pointer, dem->bandwidth, dem->srate);

		double power=(sqrt(sqr(dem->i_[0])+sqr(dem->q_[0]))+sqrt(sqr(dem->i_[0])+sqr(dem->q_[0])))/2;
		dem->power_avg=(dem->power_avg*0.9999)+power*0.0001;

		if (dem->state>=0) {
			if (dem->state<INT_MAX-1) dem->state=dem->state+1;
			//if (dem->power_avg<10) dem->state=-1; //If the power goes below a certain point, set the state to -1 => no carrier
		} else {
			//if (dem->power_avg>1000) 
			dem->state=0; //If the power goes above a certain point, set the state to 0 => carrier for 0 samples
		}

		if (dem->state<0) { //If the carrier is not on for some time (set it here) just output 1
			int res=buffer_write(dem->out, 1);
			if (res<1) {
				syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_ERR), "demodulator_fsk_demod, couldn't write to %p", dem->out);
				return res;
			}
			continue; //Go on with next sample
		}
		//The carrier is on, so we can demodulate
		//Determine derivatives
		double i_d=dem->i_[1]-dem->i_[0];
		double q_d=dem->q_[1]-dem->q_[0];
		double f_=dem->q_[0]*i_d - dem->i_[0]*q_d;
		double frq=f_/power;
		res=buffer_write(dem->out, frq*1+dem->i_[1]*0);
		if (res<1) {
			syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_ERR), "demodulator_fsk_demod, couldn't write to %p", dem->out);
			return res;
		}
	}
//	syslog(LOG_MAKEPRI(LOG_LOCAL1, LOG_DEBUG), "demodulator_fsk_demod, power %f", dem->power_avg);
	fprintf(stderr, "demodulator_fsk_demod, power %f\n", dem->power_avg);
	return 1;

}
