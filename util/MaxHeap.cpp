#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "MaxHeap.H"

inline int alloc_size(int size)
{
    if ( size ) {
        int i;
        for ( i=1; i<size; i<<=1 );
        return i;
    }
    else return 0;
}

MaxHeapBase::~MaxHeapBase()
{
    for ( int i = n_elem; i>0; i-- ) {
	delete elements[i];
    }
    delete [] elements;
}

ktype MaxHeapBase::maxkey() {
    if ( n_elem <= 0 ) abort();
    return elements[1]->key;
}

void* MaxHeapBase::maxinfo() {
    if ( n_elem <= 0 ) abort();
    return elements[1]->info;
}

void MaxHeapBase::resize(int newsize)
{
    HeapElem** newarray = new HeapElem*[newsize];
    memcpy(newarray, elements, sizeof(HeapElem*)*array_size);
    delete [] elements;
    elements = newarray;
    array_size = newsize;
}

void MaxHeapBase::insert(ktype key, void* item) // ‘}“ü‚µ‚Äupheap
{
    if ( array_size == n_elem+1 ) resize(array_size<<1);
    elements[++n_elem] = new HeapElem(key, item);
    assert( hash.seek(item) == 0 );
    hash[ elements[n_elem]->info ] = n_elem;
    upheap(n_elem);
}

void* MaxHeapBase::delmax()
{
    if ( n_elem <= 0 ) abort();
    HeapElem* max = elements[1];
    void* info = max->info;
    delete max;
    hash.del(info);
    elements[1] = elements[n_elem--];
    if ( n_elem > 0 ) {
	hash[ elements[1]->info ] = 1;
	downheap(1);
    }
    return info;
}

void MaxHeapBase::update(ktype newkey, void* item) // key‚ðXV
{
    if ( hash.seek(item) == 0 ) abort();
    int place = hash[item];
    HeapElem* elem = elements[place];
    ktype oldkey = elem->key;
    elem->key = newkey;

    if ( oldkey < newkey ) upheap(place);
    else if ( oldkey > newkey ) downheap(place);
}

void MaxHeapBase::del(void* item) // item‚Ìíœ
{
    if ( hash.seek(item) == 0 ) abort();

    int place = hash[item];
    HeapElem* elem = elements[place];
    ktype oldkey = elem->key;

    if ( place != n_elem ) {
	elements[place] = elements[n_elem];
	ktype newkey = elements[n_elem]->key;

	hash[ elements[place]->info ] = place;
	if ( oldkey < newkey ) upheap(place);
	else if ( oldkey > newkey ) downheap(place);
    }
    delete elem;
    hash.del(item);
    n_elem--;
}

ktype MaxHeapBase::itemkey(void* item) // item‚Ì’l
{
    if ( hash.seek(item) == 0 ) abort();
    int place = hash[item];
    return elements[place]->key;
}

int MaxHeapBase::exist(void* item) // item‚ª‚ ‚é‚©‚Ç‚¤‚©
{
    return (hash.seek(item) != 0);
}

void MaxHeapBase::upheap(int idx)
{
    HeapElem* orig = elements[idx];
    while ( idx > 1 ) {
	int half = idx>>1;
	if ( elements[half]->key >= orig->key ) break;
	elements[idx] = elements[half];
	hash[ elements[idx]->info ] = idx;
	idx = half;
    }
    elements[idx] = orig;
    hash[ elements[idx]->info ] = idx;
}

void MaxHeapBase::downheap(int idx)
{
    HeapElem* orig = elements[idx];
    int half_n_elem = n_elem>>1;
    while( idx <= half_n_elem ) {
	int d_idx = idx<<1;
	if ( (d_idx < n_elem) && 
	     (elements[d_idx]->key < elements[d_idx+1]->key) ) d_idx++;
	if ( orig->key >= elements[d_idx]->key ) break;
	elements[idx] = elements[d_idx];
	hash[ elements[idx]->info ] = idx;
	idx = d_idx;
    }
    elements[idx] = orig;
    hash[ elements[idx]->info ] = idx;
}

void MaxHeapBase::fprint(FILE* fp) const
{
    int n_printing = ( n_elem > 20 ) ? 20 : n_elem;
    if ( n_elem == 0 ) {
	fprintf(fp, "No element.\n");
    }
    else {
	for ( int i = 1; i<=n_printing; i++ ) fprintf(fp, "%3d ", i);
	fprintf(fp, "\n");
	for ( int i = 1; i<=n_printing; i++ )
	    fprintf(fp, "%3d ", elements[i]->key);
	fprintf(fp, "\n");
    }
}
