#ifndef HT_SEP_CHAINING_H_
#define HT_SEP_CHAINING_H_
/* hastable implementation with separate chaining (singly linked list) for hash
 * collisions */

#include <stdio.h>

typedef struct map_entry
{
  const void *key;
  int value;
  void *data;
} Ht_literal;

typedef struct elem
{
  unsigned cnt;
  char *str;  // key
  void *data; // value
  struct elem *next;
} List;

typedef struct HB
{
  size_t nlist; // number of present list elements for this bucket
  /* further down the road more metadata about a ht bucket could be put inside
   this struct
   * i.e
   pthread_mutex_t lock;  // для многопоточности
   time_t last_access;    // для LRU кэша
   size_t collisions;      // статистика коллизий
   */
  List *head; // ptr to head node of the list
} Ht_cell;

typedef struct HT
{
  size_t nKeys;    /* number of unique keys */
  size_t nbuckets; /* number of buckets allocated */
  Ht_cell *bucket; /* hashtable members - ptr to the first bucket of ht buckets
                      array */
} Hashtable;

int ht_make (Hashtable *ht);

List *ht_lookup (Hashtable *ht, const void *key);

void ht_fast_update (List **head, const Ht_literal entry);

int ht_destroy (Hashtable *ht);

int ht_get (Hashtable *ht, const void *key, const int default_value);

/* returns address of key-value pair (list item) */
List *ht_set (Hashtable *ht, const void *key);

/* Update the hashtable with the key/value(integer) pair, overwriting value if
 * key exists. Returns 0 or overwritten value(integer) if key existed. */
int ht_update (Hashtable *ht, const Ht_literal entry); // redundant maybe

// Ht_literal ht_pop(Hashtable *ht);

int ht_remove (Hashtable *ht, const void *key);

unsigned hashAscii (const char *str);
#endif
