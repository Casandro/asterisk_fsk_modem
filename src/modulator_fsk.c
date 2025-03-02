#include "modulator_fsk.h"
#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

modulator_fsk_t *modulator_fsk_create(const int srate, const int brate, const double f0, const double f1, buffer_t *b)
{
	modulator_fsk_t *f=malloc(sizeof(modulator_fsk_t));
	if (f==NULL) {
		fprintf(stderr, "modulator_fsk_create: malloc failed\n");
		return NULL;
	}
	memset(f, 0, sizeof(modulator_fsk_t));
	f->out=b;
	f->bitlen=srate/brate;
	f->omega_0=f0/srate*M_PI*2;
	f->omega_1=f1/srate*M_PI*2;
	f->samplepos=-1;
	return f;
}

int modulate_fsk_sample(modulator_fsk_t *m, const int b)
{
	if (m->phi>2*M_PI) m->phi=m->phi-2*M_PI;
	double v=sin(m->phi)*0.1;
	if (b==0) {
		m->phi=m->phi+m->omega_0;
	} else {
		m->phi=m->phi+m->omega_1;
	}
	int res=buffer_write(m->out, v);
	if (res==0) {
		fprintf(stderr, "modulate_fsk_sample buffer_write failed %p %d\n", m, b);
	}
	return res;
}


int modulator_fsk_modulate(modulator_fsk_t *m, const int len)
{
	if (m==NULL) return 0;
	for (int n=0; n<len; n++) {
		if (m->samplepos<0) {
			if (m->bit_wp!=m->bit_rp) {
				m->samplepos=0;
				m->bit=m->bit_buffer[m->bit_rp];
				m->bit_rp=(m->bit_rp+1) % MODULATOR_FSK_BLEN;
			} else {
				m->bit=1; //idle state in between bits
			}
		}
		if (m->samplepos>=0)
			m->samplepos=m->samplepos+1;
		if (m->samplepos>=m->bitlen) {
			m->samplepos=-1;
		}
		int res=modulate_fsk_sample(m, m->bit);
		if (res==0) {
			fprintf(stderr, "modulator_fsk_modulate failed %p sample: %d\n", m, n);
			return 0;
		}
	}
	return 1;
}

int modulator_fsk_bit_num(const modulator_fsk_t *m)
{
	if (m==NULL) return 0;
	int diff=m->bit_wp-m->bit_wp;
	if (diff>=0) return diff;
	return diff+MODULATOR_FSK_BLEN;
}

void modulator_fsk_queue_bit(modulator_fsk_t *m, const int bit)
{
	if (m==NULL) return;
	int nwp=(m->bit_wp+1)%MODULATOR_FSK_BLEN;
	if (nwp==m->bit_rp) {
		fprintf(stderr, "modulator_fsk_bit_queue: buffer overrun\n");
	}
	m->bit_buffer[m->bit_wp]=bit;
	m->bit_wp=nwp;
	return;
}

void modulator_fsk_queue_byte(modulator_fsk_t *m, const int byte)
{
	modulator_fsk_queue_bit(m, 0); //Start bit
	for (int n=0; n<8; n++) modulator_fsk_queue_bit(m, (byte>>n)&0x1);
	modulator_fsk_queue_bit(m, 1); //Stopp bit
}

void modulator_fsk_queue_break(modulator_fsk_t *m)
{
	for (int n=0; n<10; n++) modulator_fsk_queue_bit(m, 0);
	for (int n=0; n<4; n++) modulator_fsk_queue_bit(m, 1);
}

void modulator_fsk_queue_pause(modulator_fsk_t *m, const int pause)
{
	for (int n=0; n<pause; n++) modulator_fsk_queue_bit(m, 1);
}
