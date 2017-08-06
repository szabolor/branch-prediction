#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lru_cache.h"


void lru_cache_bring_to_front(
	struct lru_cache   *cache,
	struct cache_entry *entry
)
{
	if (entry != cache->newest_entry) {
		
		if (entry == cache->oldest_entry) {
			/* if the current entry was the oldest element */
			cache->oldest_entry = entry->newer;
			cache->oldest_entry->older = NULL;
		} else {
			/* remove element from the current position */
			entry->older->newer = entry->newer;
			entry->newer->older = entry->older;
		}

		entry->newer = NULL;
		entry->older = cache->newest_entry;

		cache->newest_entry->newer = entry;
		cache->newest_entry = entry;
	}
}


struct cache_entry * lru_cache_find_entry(
	struct lru_cache *cache,
	CACHE_KEY_TYPE find_key
)
{
	struct cache_entry *cache_ptr = cache->oldest_entry;

	/* O(n) search for the key */
	while (cache_ptr) {
		if ( CACHE_KEY_COMPARE(cache_ptr->key, find_key) ) {
			lru_cache_bring_to_front(cache, cache_ptr);
			return cache_ptr;
		}
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

	/* search for the key, or create a new one */
	cache_ptr = lru_cache_find_entry(cache, key);
	if (cache_ptr == NULL) {
		/* if not found, create a new entry */
		cache_ptr = (struct cache_entry *) malloc(sizeof(struct cache_entry));
		cache_ptr->key = key;
		cache_ptr->data = data;
		cache_ptr->older = cache->newest_entry;
		cache_ptr->newer = NULL;

		if (cache->newest_entry)
			cache->newest_entry->newer = cache_ptr;
		cache->newest_entry = cache_ptr;

		if (cache->oldest_entry == NULL)
			cache->oldest_entry = cache_ptr;

		/* if current entry length more than maximal depth remove the oldest */
		cache->current_depth += 1;
		if (cache->current_depth > cache->depth) {
			cache_ptr = cache->oldest_entry;
			cache->oldest_entry = cache_ptr->newer;
			cache->oldest_entry->older = NULL;
			free(cache_ptr);
			cache->current_depth -= 1;
		}
	} else {
		/* if found, so it must be the newest_entry, thus update that with data */
		cache->newest_entry->data = data;
	}

	return cache->newest_entry;
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

	printf("Cache [0x%08lx] with %lu entry (maximal: %lu):\n",
			(uint64_t) cache, cache->current_depth, cache->depth);
	printf("oldest: [0x%08lx], newest: [0x%08lx]\n",
			(uint64_t) cache->oldest_entry,  (uint64_t) cache->newest_entry);
	while (cache_ptr) {
		printf("[0x%08lx] (older: 0x%08lx, newer: 0x%08lx) key: %4lu data: %4u\n", 
			(uint64_t) cache_ptr, (uint64_t) cache_ptr->older,
			(uint64_t) cache_ptr->newer, cache_ptr->key, cache_ptr->data);
		cache_ptr = cache_ptr->newer;
	}
	printf("\n");
}

#if TEST_MODE
int main(int argc, char const *argv[])
{
	struct lru_cache *cache;
	struct cache_entry *entry;
	int i;

	cache = lru_cache_init(4);
	for (i=0; i<20; ++i) {
		printf("Adding key=%d, data=%d\n", ((i << 1) & 0x3) ^ ((i >> 2) & 0x3), i);
		lru_cache_insert_entry(cache, ((i << 1) & 0x3) ^ ((i >> 2) & 0x3), i);
		lru_cache_print(cache);
	}

	for (i=0; i<20; ++i) {
		printf("Accessing key=%d\n", ((i << 2) & 0x7) ^ ((i >> 1) & 0x3));
		entry = lru_cache_find_entry(cache, ((i << 2) & 0x7) ^ ((i >> 1) & 0x3));
		if (entry)
			printf("Found: [0x%08lx] (older: 0x%08lx, newer: 0x%08lx) key: %4lu data: %4u\n", 
			(uint64_t) entry, (uint64_t) entry->older,
			(uint64_t) entry->newer, entry->key, entry->data);
		else
			printf("Key not found :(\n");
		lru_cache_print(cache);
	}

	entry = lru_cache_find_entry(cache, 116);
	if (entry)
		printf("Found entry [0x%lx]: data=%u\n", (uint64_t) entry, entry->data);
	else
		printf("Key not found :(\n");

	lru_cache_destroy(cache);
	
	return 0;
}
#endif