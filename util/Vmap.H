#ifndef _Vmap_h
#define _Vmap_h 1

#include <assert.h>

#define INITCAPA 8
typedef void* Pix;
typedef unsigned int uint;

template <class K, class V>
class Vmap {
protected:
    uint	count;
    uint	size;
    V		def;
    K*		tab;
    V*		cont;
    char*       status;
    int		(*hashfunc)(K key);

public:
    Vmap(V dflt, int (*hashf)(K key), uint sz=INITCAPA);
//     Vmap(const Vmap& a);
    ~Vmap() {
	delete [] tab;
	delete [] cont;
	delete [] status;
    }

    int		length() const { return count; }// current number of items
    void	clear(); 
    void	resize(uint newsize = 0);

    V&		operator [] (K key);
    Pix		seek(K& key);
    void	del(K& key);

    Pix		first();
    void	next(Pix& i);
    K&		key(Pix i)
    	{ assert( i != 0 ); return *((K*)i); }
    V&		value(Pix i) {
	assert( i != 0 );
	return cont[((unsigned)(i) - (unsigned)(tab)) / sizeof(K)];
    }
};

#endif
