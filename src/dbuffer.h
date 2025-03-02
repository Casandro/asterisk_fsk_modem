#pragma once

#include <stdint.h>

#define DBUFFER_IDLE (-1)
#define DBUFFER_BREAK (-2)
#define DBUFFER_CARRIER_ON (-3)
#define DBUFFER_CARRIER_OFF (-4)


typedef struct {
	int rp;
	int wp;
	int len;
	int16_t *buffer;
} dbuffer_t;


dbuffer_t *dbuffer_create(const int len);
void dbuffer_free(dbuffer_t *b);

int dbuffer_put(dbuffer_t *b, int16_t w);
int16_t dbuffer_get(dbuffer_t *b);

int dbuffer_data(const dbuffer_t *b);
int dbuffer_space(const dbuffer_t *b);
