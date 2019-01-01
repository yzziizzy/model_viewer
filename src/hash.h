#ifndef __EACSMB_HASH_H__
#define __EACSMB_HASH_H__

#include <stdint.h>


struct hash_bucket {
	uint64_t hash;
	void* key;
	void* value;
};

typedef uint64_t (*hashFn_t)(void*);
typedef int (*keyCompareFn_t)(void*, void*);

typedef struct hash_table {
	size_t alloc_size;
	size_t fill;
	float grow_ratio; // default 0.75
	float shrink_ratio; // set greater than 1.0 to entirely disable, default 99.0
	struct hash_bucket* buckets;
	
	hashFn_t hashFn; 
	keyCompareFn_t keyCompareFn; 
} HashTable;

// NOTE: if you pass in garbage pointers you deserve the segfault

uint64_t HT_HashFn(void* key, size_t len);
uint64_t HT_CStringHashFn(void* key);

HashTable* HT_create(int allocPOT);
void HT_init(HashTable* ht, int allocPOT);
HashTable* HT_createCustom(int allocPOT, hashFn_t hashFn, keyCompareFn_t keyCompareFn);
void HT_destroy(HashTable* obj, int free_values_too);
int HT_resize(HashTable* obj, int newSize);
int HT_get(HashTable* obj, void* key, void** val);
int HT_set(HashTable* obj, void* key, void* val);
int HT_delete(HashTable* obj, void* key);

// iteration. no order. results undefined if modified while iterating
// returns 0 when there is none left
// set iter to NULL to start
int HT_next(HashTable* obj, void** iter, void** key, void** value);



#define HASH_FN_FOR_TYPE(name, x) \
uint64_t name(void* key) { \
	return HT_HashFn(key, sizeof(x)); \
} 


/*
// special faster version for storing just integer sets
struct int_hash_bucket {
	uint64_t key;
	uint64_t value;
};

typedef struct int_hash_table {
	size_t alloc_size;
	size_t fill;
	float grow_ratio;
	float shrink_ratio;
	struct int_hash_bucket* buckets; 
} IntHash;

*/







#endif //__EACSMB_HASH_H__
