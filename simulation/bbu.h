#ifndef _BBU_H_
#define _BBU_H_

#define BIT_TYPE uint_fast8_t

struct bbu {
	int history; /* size in bits of the shift register */
	int size;    /* numbers of the state machine lines */
	int state_count; /* number of the states of the state machine (4) */
	uint64_t shr; /* shift register with the `history'-sized branching history */ 
	struct lru_cache *cache; /* cache: key=>shift register, data=>state */
};

void binary_print(uint64_t num, uint64_t width);
void bbu_print(struct bbu *bbu);
BIT_TYPE bbu_predict(struct bbu *bbu, BIT_TYPE correct);
struct bbu * bbu_init(int history, int size, int state_count);
void bbu_destroy(struct bbu *bbu);

#endif