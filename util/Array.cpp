#include "Array.H"


static inline int alloc_size(int size)
{
    if ( size ) {
        int i;
        for ( i=1; i<size; i<<=1 );
        return i;
    }
    else return 0;
}

static inline int valid_index(int index, int size)
{
    if ( index < 0 ) return 0;
    else if ( index >= size ) return 0;
    else return 1;
}


template <class T>
Array<T>::Array(const Array<T>& obj)
    : ent(new T[alloc_size(obj.sizerep)]), sizerep(obj.sizerep) {
    for ( int i=0; i<sizerep; i++ ) ent[i] = obj.ent[i];
}

template <class T>
Array<T>::Array(int n_elem)
    : ent(new T[alloc_size(n_elem)]), sizerep(n_elem) {}

template <class T>
T& Array<T>::operator [] (int index) const
{
    assert( valid_index(index, sizerep) );
    return ent[index];
}

template <class T>
Array<T>& Array<T>::operator = (const Array<T>& obj)
{
    int old_size = alloc_size(sizerep);
    int new_size = alloc_size(obj.sizerep);;
    if ( old_size < new_size ) {
        if ( ent ) delete [] ent;
        ent = new T[new_size];
    }
    sizerep = obj.sizerep;
    for ( int i=0; i<sizerep; i++ ) ent[i] = obj.ent[i];
    return *this;
}

template <class T>
int Array<T>::operator == (const Array<T>& obj)
{
    if ( ent != obj.ent ) return 0;
    else if ( sizerep != obj.sizerep ) return 0;
    else return 1;
}

template <class T>
void Array<T>::replace(const T& deling, const T& adding)
{
    int i;
    for ( i=0; i<sizerep; i++ )
        if ( ent[i] == deling ) {
            ent[i] = adding;
            break;
        }
    assert( i < sizerep );
}
 
template <class T>
void Array<T>::del_i(int index)
{
    assert( valid_index(index, sizerep) );
    assert( sizerep );
    if ( sizerep == 1 ) {
        clear();
    }
    else {
        for ( int j=index; j<sizerep-1; j++ ) ent[j] = ent[j+1];
        sizerep--;
    }
}

template <class T>
void Array<T>::add(const T& adding)
{
    if ( ent == 0 ) {
        sizerep = 1;
        ent = new T[8];
        ent[0] = adding;
    }
    else {
        int i;
        for ( i=1; i<sizerep; i<<=1 );
        if ( sizerep == i ) {
            T* newent = new T[ 2*sizerep ];
            sizerep++;
            for ( int j=0; j<sizerep-1; j++ ) newent[j] = ent[j];
            newent[sizerep-1] = adding;
            delete [] ent;
            ent = newent;
        }
        else {
            sizerep++;
            ent[sizerep-1] = adding;
        }
    }
}

template <class T>
void Array<T>::add_a(const Array<T>& obj)
{
    int n0 = alloc_size(sizerep);
    int n1 = alloc_size(sizerep+obj.sizerep);
    int i;

    if ( n0 < n1 ) {
        T* oldent = ent;
        ent = new T[n1];
        if ( oldent ) {
            for ( i=0; i<sizerep; i++ ) ent[i] = oldent[i];
            delete [] oldent;
        }
    }
    for ( i=0; i<obj.sizerep; i++ ) ent[sizerep+i] = obj.ent[i];
    sizerep += obj.sizerep;
}

template <class T>
int Array<T>::del(const T& deling)
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

template <class T>
void Array<T>::reserve(int newsize)
{
    if ( sizerep < newsize ) {
        int old_size = alloc_size(sizerep);
        int new_size = alloc_size(newsize);
        if ( old_size == new_size ) {}
        else if ( ent ) {
            T* newent = new T[new_size];
            for ( int i=0; i<sizerep; i++ ) newent[i] = ent[i];
            delete [] ent;
            ent = newent;
        }
        else {
            ent = new T[new_size];
        }
    }
    sizerep = newsize;
}

template <class T>
int Array<T>::position(const T& item)
{
    for ( int i=0; i<sizerep; i++ ) {
        if ( ent[i] == item ) return i;
    }
    return -1;
}

template <class T>
void Array<T>::sort(int (*cmp)(const void*, const void*))
{
    qsort(ent, sizerep, sizeof(T), cmp);
}

template <class T>
void Array<T>::fill(const T& value)
{
    for ( int i=sizerep-1; i>=0; i-- ) ent[i] = value;
}

