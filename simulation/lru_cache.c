#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lru_cache.h"

struct cache_entry * lru_cache_find_entry(
	struct lru_cache *cache,
	CACHE_KEY_TYPE find_key
)
{
	struct cache_entry *cache_ptr = cache->oldest_entry;

	/* O(n) search for the key */
	while (cache_ptr) {
		if ( CACHE_KEY_COMPARE(cache_ptr->key, find_key) )
			return cache_ptr;
		cache_ptr = cache_ptr->newer;
	}

	return NULL;
}

struct cache_entry * lru_cache_insert_entry(
	struct lru_cache *cache,
	CACHE_KEY_TYPE key,
	CACHE_DATA_TYPE data
)
{
	struct cache_entry *cache_ptr;
	
	cache_ptr = (struct cache_entry *) malloc(sizeof(struct cache_entry));
	cache_ptr->key = key;
	cache_ptr->data = data;
	cache_ptr->newer = NULL;

	if (cache->newest_entry) 
		cache->newest_entry->newer = cache_ptr;

	cache->newest_entry = cache_ptr;
	cache->current_depth += 1;

	if (cache->oldest_entry == NULL)
		cache->oldest_entry = cache_ptr;

	/* Remove the oldest entry, because of the one-by-one insertion the difference is
	   0 or 1 exactly */
	if (cache->current_depth > cache->depth) {
		cache_ptr = cache->oldest_entry;
		cache->oldest_entry = cache_ptr->newer;
		free(cache_ptr);
		cache->current_depth -= 1;
	}

	return cache->newest_entry;
}

struct cache_entry * lru_cache_insert_or_update_entry(
	struct lru_cache *cache,
	CACHE_KEY_TYPE key,
	CACHE_DATA_TYPE data
)
{
	struct cache_entry *cache_ptr;

	cache_ptr = lru_cache_find_entry(cache, key);
	if (cache_ptr)
		cache_ptr->data = data;
	else
		cache_ptr = lru_cache_insert_entry(cache, key, data);

	return cache_ptr;
}

struct lru_cache * lru_cache_init(CACHE_DEPTH_TYPE depth)
{
	struct lru_cache *cache;

	cache = (struct lru_cache *) malloc(sizeof(struct lru_cache));
	cache->depth = depth;
	cache->current_depth = 0;
	cache->newest_entry = NULL;
	cache->oldest_entry = NULL;

	return cache;
}

void lru_cache_destroy(struct lru_cache *cache)
{
	struct cache_entry *cache_ptr = NULL, *cache_tmp;
	
	if (cache)
		cache_ptr = cache->oldest_entry;

	while (cache_ptr) {
		cache_tmp = cache_ptr;
		cache_ptr = cache_ptr->newer;
		free(cache_tmp);
	}

	if (cache)
		free(cache);
}

void lru_cache_print(struct lru_cache *cache)
{
	struct cache_entry *cache_ptr;
	cache_ptr = cache->oldest_entry;

	printf("Cache [0x%lx] with %lu entry (maximal: %lu):\n",
		   (uint64_t) cache, cache->current_depth, cache->depth);
	while (cache_ptr) {
		printf("%lu - %u\n", cache_ptr->key, cache_ptr->data);
		cache_ptr = cache_ptr->newer;
	}
	printf("------------\n\n");
}

#if TEST_MODE
int main(int argc, char const *argv[])
{
	struct lru_cache *cache;
	struct cache_entry *entry;

	cache = lru_cache_init(3);
	lru_cache_insert_or_update_entry(cache, 111, 221);
	lru_cache_insert_or_update_entry(cache, 112, 222);
	lru_cache_insert_or_update_entry(cache, 113, 223);
	lru_cache_insert_or_update_entry(cache, 114, 224);
	lru_cache_insert_or_update_entry(cache, 115, 225);
	lru_cache_insert_or_update_entry(cache, 116, 226);
	lru_cache_insert_or_update_entry(cache, 116, 227);
	lru_cache_insert_or_update_entry(cache, 114, 228);
	lru_cache_print(cache);

	entry = lru_cache_find_entry(cache, 116);
	if (entry)
		printf("Found entry [0x%lx]: data=%u\n", (uint64_t) entry, entry->data);
	else
		printf("Key not found :(\n");

	lru_cache_destroy(cache);
	
	return 0;
}
#endif