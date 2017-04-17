#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lru_cache.h"

/*
   Lorant SZABO
   
   Build with:
    `gcc -Wall -Wextra -pedantic -o branch_simulation branch_simulation.c lru_cache.c'
   and run like:
    `./branch_simulation 9 3 16 16 4'
   or
    `./branch_simulation 8 $(printf "%d" 0x55) 16 16 4'
*/

#define SAMPLE_SIZE (1000000)
#define BIT_TYPE uint_fast8_t

struct bbu {
	int history; /* size in bits of the shift register */
	int size;    /* numbers of the state machine lines */
	int state_count; /* number of the states of the state machine (4) */
	uint64_t shr; /* shift register with the `history'-sized branching history */ 
	struct lru_cache *cache; /* cache: key=>shift register, data=>state */
};

void binary_print(uint64_t num, uint64_t width)
{
	width = 1 << width;
	while (width >>= 1) {
		putchar('0' + ( (num & width) != 0 ) );
	}
}

void bbu_print(struct bbu *bbu)
{
	struct cache_entry *entry;

	printf("BBU [0x%lx]:\n", (uint64_t) bbu);
	printf(" - history bits: %d\n", bbu->history);
	printf(" - table size: %d\n", bbu->size);
	printf(" - state machine state count: %d\n", bbu->state_count);
	printf(" - actual shift register status: ");
	binary_print(bbu->shr, bbu->history);
	putchar('\n');

	printf("Branch-prediction table:\n");
	entry = bbu->cache->oldest_entry;
	while (entry) {
		putchar(' ');
		binary_print(entry->key, bbu->history);
		printf(" -> %u\n", entry->data);
		entry = entry->newer;
	}

	printf("-------------\n\n");
}

BIT_TYPE bbu_predict(struct bbu *bbu, BIT_TYPE correct)
{
	/* Predict the next branching based on the current shift register status */
	struct cache_entry *entry;
	BIT_TYPE retval = 0;
	
	entry = lru_cache_find_entry(bbu->cache, bbu->shr);
	if (entry == NULL)
		entry = lru_cache_insert_entry(bbu->cache, bbu->shr, (bbu->state_count>>1) );

	/* predict! */
	retval = ( entry->data > (bbu->state_count>>1) );

	/* update history with the actual branching */
	if (correct) 
		entry->data += (entry->data < (bbu->state_count - 1)) ? 1 : 0;
	else
		entry->data -= (entry->data > 0) ? 1 : 0;

	/* update branching shift register with the actual value */
	bbu->shr <<= 1;
	bbu->shr &= (1 << bbu->history) - 1;
	bbu->shr |= correct & 1;

	return retval;
}

struct bbu * bbu_init(int history, int size, int state_count)
{
	struct bbu *bbu_ptr;

	bbu_ptr = (struct bbu *) malloc(sizeof(struct bbu));
	bbu_ptr->history = history;
	bbu_ptr->size = size;
	bbu_ptr->shr = 0;
	bbu_ptr->state_count = state_count;
	bbu_ptr->cache = lru_cache_init(size);

	return bbu_ptr;
}

void bbu_destroy(struct bbu *bbu)
{
	if (bbu && bbu->cache)
		lru_cache_destroy(bbu->cache);
	if (bbu)
		free(bbu);

}

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