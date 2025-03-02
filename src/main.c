#include <stdio.h>
#include <stdint.h>

#include "buffer.h"
#include "modulator_fsk.h"
#include "demodulator_fsk.h"
#include "resample.h"


int handle_data(uint8_t *data, const int len)
{
	for (int n=0; n<len; n++) printf("%hhn ", data);
	return 0;
}


void dump_buffer(buffer_t *b)
{
	int len=buffer_data(b);
	for (int n=0; n<len; n++) {
		double v=0;
		int res=buffer_read(b, &v);
		if (res==0) {
			fprintf(stderr, "dump_buffer: buffer_read failed\n");
			return;
		}
		printf("%d %10lf\n", 0, v);
	}
}



void test()
{
	int frq_1= 1300;
	int frq_0=2100;
	int brate=1200;
	buffer_t *modulated=buffer_create(1024, 24000);
	buffer_t *modulated_8k=buffer_create(1024, 8000);
	buffer_t *modulated_upsampled=buffer_create(1024, 24000);
	buffer_t *demodulated=buffer_create(1024, 24000);
	resampler_t *resampler_24k_8k=resampler_create(modulated, modulated_8k);
	resampler_t *resampler_8k_24k=resampler_create(modulated_8k, modulated_upsampled);
	modulator_fsk_t *modulator=modulator_fsk_create(brate, frq_0, frq_1, modulated);
	modulator_fsk_start(modulator);
	demodulator_fsk_t *demodulator=demodulator_fsk_create(modulated_upsampled, demodulated, frq_0, frq_1, brate);
	modulator_fsk_queue_pause(modulator, brate/2);
	modulator_fsk_queue_break(modulator);
	modulator_fsk_queue_pause(modulator, brate/2);
	for (int b=0; b<16; b++) {
		modulator_fsk_queue_pause(modulator, 20);
		for (int n=0; n<64; n++) {
			int bit_num=(n%4);
			modulator_fsk_queue_bit(modulator, ((~b)>>bit_num)%2);
		}
	}
	modulator_fsk_queue_pause(modulator, brate/4);
	for (int n=32; n<127; n++) modulator_fsk_queue_byte(modulator, n);
	for (int n=0; n<100*1024/1024*16; n++) {
		modulator_fsk_modulate(modulator, buffer_space(modulated));
		resampler_resample(resampler_24k_8k);
		resampler_resample(resampler_8k_24k);
		demodulator_fsk_demod(demodulator);
		dump_buffer(demodulated);
	}
}



int main(int argc, char *argv[])
{
	if (argc>0)
		fprintf(stderr, "Wurst: %s\n", argv[0]);
	test();
	return 0;
	while (!feof(stdin)) {
		uint8_t buffer[256*256];
		fread(buffer, 3, 1, stdin);
		int type=buffer[0];
		int size=buffer[1] + (buffer[2]<<8);
		fread(buffer, size, 1, stdin);
		switch(type) {
			case 0x00:
				fprintf(stderr, "Connection close requested\n");
				return 0;
				break;
			case 0x01:
				fprintf(stderr, "UUID: ");
				for (int n=0; n<size; n++) fprintf(stderr, "%02x", buffer[n]);
				fprintf(stderr, "\n");
				break;
			case 0x10:
				//Handle audio data from socket
				handle_data(buffer, size);
				break;
			case 0xff:
				fprintf(stderr, "Error: ");
				for (int n=0; n<size; n++) fprintf(stderr, "%c", buffer[n]);
				break;
		}
	}
}
