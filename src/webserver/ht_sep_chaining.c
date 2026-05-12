#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ht_sep_chaining.h"

/* hastable implementation with separate chaining (singly linked list) for hash
 * collisions */
/*
 * хэш функция для хранения словаря уникальных слов в файле:
 * если слово это строка вида w[n],
 * то хеш этого слова будет: ((w[0] * (i + 1) + ... w[n-1] * (i + 1)) / n + 1)
 * % NBUCKETS
 */

int
hashAscii (const char *str)
{
  const size_t len = strlen (str) + 1;
  long hash = 0;
  if (str == NULL)
    return -1;
  for (size_t i = 0; i < len - 1; ++i)
    {
      hash += (int)str[i] * (1 + i);
      // hash += (int)str[i] / 1+i;
    }
  // assert(len), assert(hash);
  return hash / len;
}

List **
wl_find (const char *str, List **head)
{
#ifdef DEBUG
  assert (str);
#endif
  List **ptelem;

  /* линейный поиск слова в списке */
  for (ptelem = head; *ptelem != NULL; ptelem = &(*ptelem)->next)
    {
#ifdef DEBUG
      assert ((*ptelem)->str);
#endif
      if (strcmp ((*ptelem)->str, str) == 0)
        break;
    }

  return ptelem;
}

void
wl_insert_head (const char *src, List **head)
{
#ifdef DEBUG
  assert (src && head);
#endif
  /* создание нового элемента списка с добавлением в него строки src */
  List *new = (List *)malloc (sizeof (List));
#ifdef DEBUG
  assert (new);
#endif

  new->str = (char *)malloc (1 + strlen (src));
#ifdef DEBUG
  assert (new->str);
#endif
  memcpy (new->str, src, strlen (src) + 1);
  /* ракировочка */
  new->next = *head; // записываем адрес старой ноды head в ново-созданную ноду
  *head = new; // теперь перезаписываем адрес старой ноды , чтоб он указывал на
               // ново-созданную нашу ноду
}

void
wl_append (const char *src, List **head)
{
#ifdef DEBUG
  assert (src);
#endif
  List **end = head;
  /* проход в конец списка */
  for (; *end != NULL; end = &(*end)->next)
    ;

  /* создание нового элемента списка с добавлением в него строки src */
  List *new = (List *)malloc (sizeof (List));
#ifdef DEBUG
  assert (new);
#endif
  /* strndup сама выделяет память под строку на хипе (POSIX)
   * и возвращает указатель на такую же новую строку в памяти  */
  //	new->str = strndup(src, len);

  /* но используем ANSI compliant функции*/
  new->str = malloc (1 + strlen (src));
  memcpy (new->str, src, strlen (src));
  new->cnt = 1;
  new->next = NULL;

  *end = new;
  // (*end)->w_uniq++; // а также счетчик уникальных слов в bucket
}

/* destroys all elements of list
 * returns sum of all values from all keys in list or 0 if the list was empty*/
int
wl_clean (List **list)
{
  if (!list || (*list) == NULL)
    return 0;

  int sum = (*list)->cnt;
  sum += wl_clean (&(*list)->next);

  if ((*list)->str)
    free ((*list)->str);

  free (*list), *list = NULL;

  return sum;
}

int
wl_pop (List **ptelem, const int default_val)
{
#ifdef DEBUG
  assert (*ptelem);
#endif
  int wc = default_val;
  if (*ptelem)
    {
      List *old = *ptelem;
      *ptelem = (*ptelem)->next;
      wc = old->cnt;
      free (old);
    }

  return wc;
}

int
ht_make (Hashtable *ht)
{
  const size_t nbuckets = 1000;
  ht->bucket = (Ht_cell *)calloc (nbuckets, sizeof (Ht_cell));
  if (!ht->bucket)
    return -1;
  ht->nbuckets = nbuckets, ht->nKeys = 0;

  return 0;
}

List *
ht_set (Hashtable *ht, const void *key)
{

  const int i = hashAscii ((const char *)key) % ht->nbuckets;
  List *found = *wl_find ((const char *)key, &(ht->bucket[i].head));

  if (!found)
    {
      wl_insert_head ((const char *)key, &(ht->bucket[i].head));
      found = ht->bucket[i].head;
      ht->nKeys++, ht->bucket[i].nlist++;
    }

  return found;
}

List *
ht_lookup (Hashtable *ht, const void *key)
{
#ifdef DEBUG
  assert (hashAscii ((char *)key) != -1);
#endif

  const int i = hashAscii ((const char *)key) % ht->nbuckets;
  List *if_found = *wl_find ((const char *)key, &(ht->bucket[i].head));

  return if_found;
}

int
ht_get (Hashtable *ht, const void *key, const int default_value)
{
  if (!ht || !ht->bucket)
    return default_value;

  List *found = ht_lookup (ht, key);
  if (!found)
    return default_value;

  // printf("hash: %i, bucket idx after hashing: %i\n",hash, i);  // debug
  return found->cnt;
}

/* Update the hashtable with the key/value pair, overwriting value if key
 * exists. Returns 0 or overwritten value if key existed. */
int
ht_update (Hashtable *ht, const Ht_literal entry)
{
#ifdef DEBUG
  assert (ht && ht->bucket);
  assert (ht->nbuckets);
#endif

#ifdef DEBUG
  assert (hashAscii ((const char *)entry.key) != -1);
#endif
  const int i = hashAscii ((const char *)entry.key) % ht->nbuckets;

  List **found = wl_find ((char *)entry.key, &(ht->bucket[i].head));
  unsigned old_val = 0;
  if (*found)
    {
      old_val = (*found)->cnt;
      (*found)->cnt = entry.value;
    }
  else
    {
      wl_insert_head ((const char *)entry.key, &(ht->bucket[i].head));
      ht->bucket[i].nlist++;
      ht->nKeys++;
    }

  return old_val;
}

int
ht_remove (Hashtable *ht, const void *key)
{
  // if (!ht || !ht->bucket) return -1;
#ifdef DEBUG
  assert (ht && ht->bucket);
#endif

  /* recomputing hash every time */
  const int i = hashAscii ((char *)key) % ht->nbuckets;
#ifdef DEBUG
  assert (i >= 0 && i <= (int)ht->nbuckets - 1);
#endif

  List **found = wl_find (key, &(ht->bucket[i].head));
  if (found)
    --ht->bucket[i].nlist, --ht->nKeys;

  return wl_pop (found, -1);
}

int
ht_destroy (Hashtable *ht)
{
  if (!ht || !ht->bucket)
    return -1;

  size_t matches = 0;
  for (size_t i = 0; i < ht->nbuckets; ++i)
    {
      matches += wl_clean (&(ht->bucket[i].head));
    }

  free (ht->bucket), ht->bucket = NULL;
  ht = NULL;

  return matches;
}
