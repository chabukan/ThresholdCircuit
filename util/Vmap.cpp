#include "Vmap.H"

/* codes for status fields */

#define EMPTYCELL   0
#define VALIDCELL   1
#define DELETEDCELL 2

// HASHTABLE_TOO_CROWDED(COUNT, SIZE) is true iff a hash table with COUNT
// elements and SIZE slots is too full, and should be resized.
// This is so if available space is less than 1/8.

#define CROWDED(COUNT, SIZE) ((SIZE) - ((SIZE) >> 3) <= (COUNT))

/* 
 * hashing method: double hash based on high bits of hash fct,
 * followed by linear probe. Can't do too much better if table
 * sizes not constrained to be prime.
*/
static inline uint doublehashinc(uint h, uint s)
{
  uint dh =  ((h / s) % s);
  return (dh > 1)? dh : 1;
}

template <class K, class V>
// Vmap<K,V>::Vmap(V dflt, int (*hashf)(K& key), uint sz=INITCAPA)
Vmap<K,V>::Vmap(V dflt, int (*hashf)(K key), uint sz)
: size(sz), def(dflt), hashfunc(hashf)
{
  count = 0;
  tab = new K[size];
  cont = new V[size];
  status = new char[size];
  for (uint i = 0; i < size; ++i) status[i] = EMPTYCELL;
}

template <class K, class V>
V& Vmap<K,V>::operator [](K item)
{
  if (CROWDED(count, size)) resize();

  uint bestspot = size;
  uint hashval = (*hashfunc)(item);
  uint h = hashval % size;
  for (uint i = 0; i <= size; ++i)
  {
    if (status[h] == EMPTYCELL)
    {
      ++count;
      if (bestspot >= size) bestspot = h;
      tab[bestspot] = item;
      status[bestspot] = VALIDCELL;
      cont[bestspot] = def;
      return cont[bestspot];
    }
    else if (status[h] == DELETEDCELL)
    {
      if (bestspot >= size) bestspot = h;
    }
    else if ( (tab[h] == item) )
      return cont[h];

    if (i == 0)
      h = (h + doublehashinc(hashval, size)) % size;
    else if (++h >= size)
      h -= size;
  }

  ++count;
  status[bestspot] = VALIDCELL;
  tab[bestspot] = item;
  cont[bestspot] = def;
  return cont[bestspot];
}


template <class K, class V>
Pix Vmap<K,V>::first()
{
  for (uint pos = 0; pos < size; ++pos)
    if (status[pos] == VALIDCELL) return Pix(&tab[pos]);
  return 0;
}

template <class K, class V>
void Vmap<K,V>::next(Pix& i)
{
  if (i == 0) return;
  uint pos = ((unsigned)i - (unsigned)tab) / sizeof(K) + 1;
  for (; pos < size; ++pos)
    if (status[pos] == VALIDCELL)
    {
      i = Pix(&tab[pos]);
      return;
    }
  i = 0;
}
template <class K, class V>
Pix Vmap<K,V>::seek(K& key)
{
  uint hashval = (*hashfunc)(key);
  uint h = hashval % size;
  for (uint i = 0; i <= size; ++i)
  {
    if (status[h] == EMPTYCELL)
      return 0;
    else if (status[h] == VALIDCELL && (key == tab[h]))
      return Pix(&tab[h]);
    if (i == 0)
      h = (h + doublehashinc(hashval, size)) % size;
    else if (++h >= size)
      h -= size;
  }
  return 0;
}

template <class K, class V>
void Vmap<K,V>::del(K& key)
{
  uint hashval = (*hashfunc)(key);
  uint h = hashval % size;
  for (uint i = 0; i <= size; ++i)
  {
    if (status[h] == EMPTYCELL)
      return;
    else if (status[h] == VALIDCELL && (key == tab[h]))
    {
      status[h] = DELETEDCELL;
      --count;
      return;
    }
    if (i == 0)
      h = (h + doublehashinc(hashval, size)) % size;
    else if (++h >= size)
      h -= size;
  }
}

template <class K, class V>
void Vmap<K,V>::clear()
{
  for (uint i = 0; i < size; ++i) status[i] = EMPTYCELL;
  count = 0;
}

template <class K, class V>
void Vmap<K,V>::resize(uint newsize)
{
  if (newsize <= count)
  {
    newsize = INITCAPA;
    while (CROWDED(count, newsize)) newsize <<= 1;
  }
  K* oldtab = tab;
  V* oldcont = cont;
  char* oldstatus = status;
  uint oldsize = size;
  tab = new K[size = newsize];
  cont = new V[size];
  status = new char[size];
  uint i;
  for ( i = 0; i < size; ++i) status[i] = EMPTYCELL;
  count = 0;
  for ( i = 0; i < oldsize; ++i) 
    if (oldstatus[i] == VALIDCELL) 
      (*this)[oldtab[i]] = oldcont[i];
  delete [] oldtab;
  delete [] oldcont;
  delete [] oldstatus;
}
