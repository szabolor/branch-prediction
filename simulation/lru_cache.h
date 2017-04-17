#ifndef _LRU_CACHE_H_
#define _LRU_CACHE_H_

#define TEST_MODE ( 0 )

#define CACHE_DATA_TYPE  uint_fast8_t
#define CACHE_KEY_TYPE   uint64_t
#define CACHE_DEPTH_TYPE uint_fast16_t
#define CACHE_KEY_COMPARE(A, B) ( (A) == (B) )

struct cache_entry {
	CACHE_KEY_TYPE  key;
	CACHE_DATA_TYPE data;
	struct cache_entry *newer;
};

struct lru_cache {
	CACHE_DEPTH_TYPE depth;
	CACHE_DEPTH_TYPE current_depth;
	struct cache_entry *oldest_entry;
	struct cache_entry *newest_entry;
};


struct cache_entry * lru_cache_find_entry(
	struct lru_cache *cache,
	CACHE_KEY_TYPE find_key
);
struct cache_entry * lru_cache_insert_entry(
	struct lru_cache *cache,
	CACHE_KEY_TYPE key,
	CACHE_DATA_TYPE data
);
struct cache_entry * lru_cache_insert_or_update_entry(
	struct lru_cache *cache,
	CACHE_KEY_TYPE key,
	CACHE_DATA_TYPE data
);
struct lru_cache * lru_cache_init(CACHE_DEPTH_TYPE depth);
void lru_cache_destroy(struct lru_cache *cache);
void lru_cache_print(struct lru_cache *cache);


#endif