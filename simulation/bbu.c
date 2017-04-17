#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "lru_cache.h"
#include "bbu.h"

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
	
	/* find entry, if there's one with the current history shr,
	   or create one with weak (not) taken status from the last branch */
	entry = lru_cache_find_entry(bbu->cache, bbu->shr);
	if (entry == NULL)
		entry = lru_cache_insert_entry(bbu->cache, bbu->shr, (bbu->state_count>>1) + (bbu->shr&1) );

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