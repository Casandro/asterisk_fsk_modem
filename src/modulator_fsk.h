#pragma once

#include "buffer.h"

#include <stdint.h>

#define MODULATOR_FSK_BLEN (1024*16)

typedef struct {
	double phi;
	int bitlen;
	double omega_0;
	double omega_1;
	int samplepos;
	int bit_wp;
	int bit_rp;
	buffer_t *out;
	int8_t bit;
	int8_t bit_buffer[MODULATOR_FSK_BLEN];
	int state;
} modulator_fsk_t;


modulator_fsk_t *modulator_fsk_create(const int brate, const double f0, const double f1, buffer_t *b);
void modulator_fsk_free(modulator_fsk_t *f);

void modulator_fsk_start(modulator_fsk_t *f);

int modulator_fsk_modulate(modulator_fsk_t *m, const int len);
int modulator_fsk_bit_num(const modulator_fsk_t *m);
void modulator_fsk_queue_bit(modulator_fsk_t *m, const int bit);
void modulator_fsk_queue_byte(modulator_fsk_t *m, const int byte);
void modulator_fsk_queue_break(modulator_fsk_t *m);
void modulator_fsk_queue_pause(modulator_fsk_t *m, const int pause);
