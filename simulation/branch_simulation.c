#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bbu.h"

/*
   Lorant SZABO
   
   Build with:
    `gcc -Wall -Wextra -pedantic -o branch_simulation branch_simulation.c bbu.c lru_cache.c'
   and run like:
    `./branch_simulation 9 3 16 16 4'
   or
    `./branch_simulation 8 $(printf "%d" 0x55) 16 16 4'
*/

#define SAMPLE_SIZE (1000000)


int main(int argc, char const *argv[])
{
	struct bbu *bbu;
	int i;
	uint_fast32_t mismatch_count;
	BIT_TYPE pred_val, correct;
	uint64_t shr;
	int shr_len = 32;
	int shr_len_minus_1 = 31;
	uint64_t mask = (1UL << shr_len) - 1;
	int bbu_history, bbu_size, bbu_state;

	if (argc == 6) {
		shr_len = atoi(argv[1]);
		shr = atol(argv[2]);
		shr_len_minus_1 = shr_len - 1;
		if (shr_len == 64)
			mask = 0xffffffffffffffffUL;
		else
			mask = (1UL << shr_len) - 1;
		shr &= mask;

		bbu_history = atoi(argv[3]);
		bbu_size = atol(argv[4]);
		bbu_state = atol(argv[5]);
	} else {
		printf("Wrong arguments!\n");
		printf("%s <gen_shr_len> <shr_init_val> <BBU_history> <BBU_size> <BBU_state>\n", argv[0]);
		exit(-1);
	}

	bbu = bbu_init(bbu_history, bbu_size, bbu_state);

	mismatch_count = 0;
	for (i=0; i<SAMPLE_SIZE; ++i) {
		/* generate a periodically repetationing pattern */
		shr = ( shr >> (shr_len_minus_1) ) | ( shr << 1 );
		shr &= mask;
		correct = (BIT_TYPE) (shr & 1);
		pred_val = bbu_predict(bbu, correct);
		mismatch_count += (pred_val != correct);
		/*printf("(%d, %d)\n", correct, pred_val);*/
	}
	printf("mismatch count = %lu\n", mismatch_count);
	printf("mismatch rate = %.3f%%\n\n", 100 * mismatch_count / (double) SAMPLE_SIZE);

	bbu_print(bbu);

	bbu_destroy(bbu);

	return 0;
}