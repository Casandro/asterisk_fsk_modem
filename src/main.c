#include <stdio.h>
#include <stdint.h>

#include "buffer.h"
#include "modulator_fsk.h"
#include "resample.h"


int handle_data(uint8_t *data, const int len)
{
	for (int n=0; n<len; n++) printf("%hhn ", data);
	return 0;
}


void dump_buffer(buffer_t *b)
{
	int len=buffer_data(b);
	printf("Length %d\n", len);
	for (int n=0; n<len; n++) {
		double v=0;
		int res=buffer_read(b, &v);
		if (res==0) {
			fprintf(stderr, "dump_buffer: buffer_read failed\n");
			return;
		}
		printf("%d %lf\n", n, v);
	}
}



void test()
{
	buffer_t *modulated=buffer_create(1024);
	buffer_t *modulated_8k=buffer_create(1024);
	buffer_t *modulated_upsampled=buffer_create(1024);
	resampler_t *resampler_24k_8k=resampler_create(3, 1, modulated, modulated_8k);
	resampler_t *resampler_8k_24k=resampler_create(1, 3, modulated_8k, modulated_upsampled);
	modulator_fsk_t *modulator=modulator_fsk_create(24000, 1200, 1300, 2100, modulated);
	modulator_fsk_queue_pause(modulator, 20);
	modulator_fsk_queue_break(modulator);
	for (int n=32; n<127; n++) modulator_fsk_queue_byte(modulator, n);
	for (int n=0; n<100; n++) {
		modulator_fsk_modulate(modulator, buffer_space(modulated));
		resampler_resample(resampler_24k_8k);
		resampler_resample(resampler_8k_24k);
		dump_buffer(modulated_upsampled);
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
