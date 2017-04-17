#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bbu.h"

/*
   Lorant SZABO
   
   Build with:
    `gcc -Wall -Wextra -pedantic -o search_patterns search_patterns.c bbu.c lru_cache.c'
   and run like:
    `./search_patterns 9'
*/

#define SAMPLE_SIZE (10000)

int main(int argc, char const *argv[])
{
	struct bbu *bbu;
	int i;
	uint_fast32_t mismatch_count;
	BIT_TYPE pred_val, correct;
	uint64_t shr, shr_init;
	int shr_len, shr_len_minus_1;
	uint64_t mask, limit;
	int bbu_history, bbu_size, bbu_state;

	if (argc == 5) {
		shr_len = atoi(argv[1]);
		shr_len_minus_1 = shr_len - 1;
		if (shr_len == 64)
			mask = 0xffffffffffffffffUL;
		else
			mask = (1UL << shr_len) - 1;
		limit = ( 1UL << (shr_len - 1) );

		bbu_history = atoi(argv[2]);
		bbu_size = atol(argv[3]);
		bbu_state = atol(argv[4]);
	} else {
		printf("Wrong arguments!\n");
		printf("Usage: %s <gen_shr_len> <BBU_history> <BBU_size> <BBU_state>\n", argv[0]);
		exit(-1);
	}

	/* test every odd number as shift register value, 
	   to test every group arrangement of 0s and 1s */
	for (shr_init=1; shr_init<limit; shr_init+=2) {
		bbu = bbu_init(bbu_history, bbu_size, bbu_state);
		shr = shr_init;
		mismatch_count = 0;
		for (i=0; i<SAMPLE_SIZE; ++i) {
			/* generate a periodically repetationing pattern */
			correct = (BIT_TYPE) (shr & 1);
			pred_val = bbu_predict(bbu, correct);
			mismatch_count += (pred_val != correct);
			shr = ( shr >> (shr_len_minus_1) ) | ( shr << 1 );
			shr &= mask;
		}
		/*printf("0x%0*lx; %lu\n", ((shr_len + 3) >> 2), shr, mismatch_count);*/
		printf("%lu; %lu\n", shr, mismatch_count);
		bbu_destroy(bbu);
	}

	return 0;
}