#include "arrayint.h"
#include <stdio.h>
#include <memory.h>

int alloc_size(int size)
{
    if ( size ) {
        int i;
        for ( i=1; i<size; i<<=1 );
        return i;
    }
    else return 0;
}

inline int valid_index(int index, int size)
{
    if ( index < 0 ) return 0;
    else if ( index >= size ) return 0;
    else return 1;
}

Arrayint::Arrayint(const Arrayint& obj)
    : ent( (int*)malloc(alloc_size(obj.sizerep)*sizeof(int)) ),
      sizerep(obj.sizerep) {
    memcpy(ent, obj.ent, sizeof(int)*sizerep);
}

Arrayint::Arrayint(int size, int* array)
    : ent( (int*)malloc(alloc_size(size)*sizeof(int)) ), sizerep(size) {
    memcpy(ent, array, sizeof(int)*sizerep);
}

Arrayint::Arrayint(int newsize)
    : ent( (int*)calloc(alloc_size(newsize), sizeof(int)) ), sizerep(newsize) {
}

int& Arrayint::operator [] (int index) const
{
    assert( valid_index(index, sizerep) );
    return ent[index];
}

Arrayint& Arrayint::operator = (const Arrayint& obj)
{
    int old_size = alloc_size(sizerep);
    int new_size = alloc_size(obj.sizerep);;
    if ( old_size < new_size ) {
        free(ent);
        ent = (int*)malloc(new_size*sizeof(int));
    }
    sizerep = obj.sizerep;
    memcpy(ent, obj.ent, sizeof(int)*sizerep);
    return *this;
}

int Arrayint::operator == (const Arrayint& obj)
{
    if ( sizerep != obj.sizerep ) return 0;
    for ( int i = sizerep-1; i>=0; i-- ) {
        if ( ent[i] != obj.ent[i] ) return 0;
    }
    return 1;
}

void Arrayint::replace(int deling, int adding)
{
    int i;
    for ( i=0; i<sizerep; i++ )
        if ( ent[i] == deling ) {
            ent[i] = adding;
            break;
        }
    assert( i < sizerep );
}
 
void Arrayint::del_i(int index)
{
    assert( valid_index(index, sizerep) );
    assert( sizerep );
    if ( sizerep == 1 ) clear();
    else {
        for ( int j=index; j<sizerep-1; j++ ) ent[j] = ent[j+1];
        sizerep--;
    }
}

void Arrayint::add(int adding)
{
    if ( sizerep == 0 ) { // ëÂÇ´Ç≥Ç0Ç©ÇÁ1Ç…
        free(ent);
        // 	if ( ent ) free(ent);
        ent = (int*)malloc(1*sizeof(int));
    }
    else { // sizerepÇ™2^nÇÃéûÇÕÅCëÂÇ´Ç≥ÇsizerepÇ©ÇÁsizerep*2Ç…
        int i;
        for ( i=1; i<sizerep; i<<=1 );
        if ( sizerep == i ) {
            int* newent = (int*)malloc(2*sizerep*sizeof(int));
            memcpy(newent, ent, sizeof(int)*sizerep);
            free(ent);
            ent = newent;
        }
    }
    ent[sizerep++] = adding;
}

void Arrayint::add_a(const Arrayint& obj)
{
    int n0 = alloc_size(sizerep);
    int n1 = alloc_size(sizerep+obj.sizerep);

    if ( n0 < n1 ) {
        int* oldent = ent;
        ent = (int*)malloc(n1*sizeof(int));
        // 	if ( oldent ) {
	    memcpy(ent, oldent, sizeof(int)*sizerep);
	    free(oldent);
        // 	}
    }
    memcpy(ent+sizerep, obj.ent, sizeof(int)*obj.sizerep);
    sizerep += obj.sizerep;
}

int Arrayint::del(int deling)
{
    assert( sizerep );
    if ( sizerep == 1 ) {
        assert( ent[0] == deling );
        clear();
        return 0;
    }
    else {
        int i;
        for ( i=0; i<sizerep; i++ ) if ( ent[i] == deling ) break;
        int delix = i;
        assert( delix < sizerep );
        for ( int j=i; j<sizerep-1; j++ ) ent[j] = ent[j+1];
        sizerep--;
        return delix;
    }
}

void Arrayint::reserve(int newsize)
{
    if ( sizerep < newsize ) {
        int old_size = alloc_size(sizerep);
        int new_size = alloc_size(newsize);
        if ( old_size == new_size ) {}
        // 	else if ( ent ) {
	    int* newent = (int*)malloc(new_size*sizeof(int));
	    memcpy(newent, ent, sizeof(int)*sizerep);
	    free(ent);
	    ent = newent;
        // 	}
        // 	else ent = (int*)malloc(new_size*sizeof(int));
    }
    sizerep = newsize;
}

int Arrayint::position(int item)
{
    for ( int i=0; i<sizerep; i++ ) {
        if ( ent[i] == item ) return i;
    }
    return -1;
}

void Arrayint::fill(int value)
{
    for ( int i=sizerep-1; i>=0; i-- ) ent[i] = value;
}

void Arrayint::print()
{
    int i;
    for ( i=0; i<sizerep; i++ ) {
        fprintf(stderr, "%2d ", ent[i]);
        if ( i % 20 == 19 ) fprintf(stderr, "\n");
    }
    if ( --i % 20 != 19 ) fprintf(stderr, "\n");
}

Arrayint Arrayint::inverse()
{
    int i, n = size();
    Arrayint result(n);
    for ( i=n; --i>=0; ) result[ (*this)[i] ] = i;
    return result;
}

void Arrayint::sort(int (*cmp)(const void* a, const void* b))
{
    qsort(ent, sizerep, sizeof(int), cmp);
}

void Arrayint::jumpup(int i, int j)
{
    assert( i <= j );
    if ( i < j ) {
        int tmp = ent[i];
        for ( int k=i; k<j; k++ ) ent[k] = ent[k+1];
        ent[j] = tmp;
    }
}

void Arrayint::jumpdown(int i, int j)
{
    assert( i >= j );
    if ( i > j ) {
        int tmp = ent[i];
        for ( int k=i; k>j; k-- ) ent[k] = ent[k-1];
        ent[j] = tmp;
    }
}

void Arrayint::jumpud(int i, int j)
{
    if ( i < j ) jumpup(i,j);
    else if ( i > j ) jumpdown(i,j);
}

// indexà»è„ÇÃïîï™Ç degree ÇæÇØÇ∏ÇÁÇ∑
// degreeÇ™ê≥ÇÃèÍçáÅCå≥ÇÃèÍèäÇ…ÇÕdefvÇ™ì¸ÇÍÇÁÇÍÇÈ
void Arrayint::shift(int index, int degree, int defv)
{
    int n = size();
    if ( degree > 0 ) {
        reserve( n + degree );
        for ( int i=n-1; i >= index; i-- ) {
            ent[i+degree] = ent[i];
            ent[i] = defv;
        }
    }
    if ( degree < 0 ) {
        assert( index+degree >= 0 );
        // indexñ¢ñûÇÃïîï™ÇÕè„èëÇ´Ç≥ÇÍÇÈÇ©Ç‡ÇµÇÍÇ»Ç¢
        for ( int i=index; i<n; i++ ) ent[i+degree] = ent[i];
        reserve( n + degree );
    }
}

// Arrayint& Arrayint::changeOrder(const Arrayint& permutation)
void Arrayint::changeOrder(const Arrayint& permutation)
{
    int n = permutation.size();
    Arrayint localpermu(permutation);

    // ëÂÇ´Ç¢ï˚Ç©ÇÁåàÇﬂÇƒÇ¢Ç≠
    for ( int dest=n; --dest>0; ) for ( int idx=dest; --idx>0; ) {
            if ( localpermu[idx] == dest ) {
                if ( idx != dest ) {
                    //              printf("jumpup( %d, %d )\n", idx, dest);
                    jumpup(idx, dest);
                    localpermu.jumpup(idx, dest);
                }
                break;
            }
        }
    for ( int idx=n; --idx>0; ) assert( localpermu[idx] == idx );
    // 
    //     return *this;
}
