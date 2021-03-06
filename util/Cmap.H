#ifndef _Cmap_h
#define _Cmap_h 1

#include <assert.h>
#include <cstdint>

#define INITCAPA 8
typedef void* Pix;
typedef unsigned int uint;

template <class K, class V>
struct KVNode
{
    K			hd;
    V			cont;
    KVNode<K,V>*	tl;

    KVNode(const K& h, const V& c, KVNode<K,V>* t=0) 
        : hd(h), cont(c), tl(t) {}
};

template <class K, class V>
class Cmap {
public:
    Cmap(V dflt, int (*hashf)(const K& key), uint sz=INITCAPA); // $B%^%C%W@8@.(B -> $B%3%s%9%H%i%/%?(B 
    Cmap(const Cmap& a); // a$B$H$$$&%^%C%W$r%3%T!<(B -> $B%3%s%9%H%i%/%?(B
    ~Cmap() { clear(); delete [] tab; } // $B:o=|(B -> clear()
    
    int		size() const { return count; }// current number of items //$B%^%C%W$NMWAG?t$rJV$9(B -> size()
    void	clear(); // clear()
    void	resize();// $B%5%$%:$r#2G\$K3HD%$9$k(B -> $BITI,MW!)(B

    Cmap&       operator = (const Cmap& a); // $BBeF~(B -> $B%3%s%9%H%i%/%?(B
    V&		operator [] (const K& key); // $BL5$$$H$-$O!$(Bdef$B$X$N(Bmap$B$r:n$k(B // $B$=$N(Bkey$B$,4IM}$9$kCM$rJV$9(B -> find()
    V		get(const K& key) const;    // $BL5$$$H$-$O!$(Bdef$B$N%3%T!<$rJV$9!%(B// $B$=$N(Bkey$B$,4IM}$9$kCM$rJV$9(B -> find()
    // v.find(key)->second or v[key]
    Pix		seek(const K& key) const; // $B$=$N(Bkey$B$,4IM}$9$k%"%I%l%9$rJV$9(B -> equal_range()
    // m.count(n) == ?/ $B$GBeMQ(B
    void	del(const K& key); // 

    Pix		first() const; // Cmap$B$N@hF,$r;X$9%"%I%l%9$rJV$9(B -> begin()
    void	next(Pix& i) const; // $B:#;X$7$F$$$k%"%I%l%9$N<!$N%"%I%l%9$rJV$9(B -> itrator++
    K&		key(Pix p) const // $B:#;X$7$F$$$k%"%I%l%9$N(Bkey$B$rJV$9(B -> iterator.first()
    { assert( p != 0 ); return ((KVNode<K,V>*)p)->hd; }
    V&		value(Pix p) const // $B:#;X$7$F$$$k%"%I%l%9$N(Bvalue$B$rJV$9(B -> iterator.second()
    { assert( p != 0 ); return ((KVNode<K,V>*)p)->cont; }

private:
    uint		count;
    uint		tab_size; 
    V			def;
    KVNode<K,V>**	tab;
    int         	(*hashfunc)(const K& key);

    // HASHTABLE_TOO_CROWDED(COUNT, SIZE) is true iff a hash table with COUNT
    // elements and SIZE slots is too full, and should be resized.
    // This is so if available space is less than 1/8.

    int crowded() { return (tab_size - (tab_size>>3) <= count); }

    // The nodes are linked together serially via a version
    // of a trick used in some vtables: odd pointers are
    // actually links to the next table entry. 
    // Not terrible, but not wonderful either

    // Cmap$B$N%"%I%l%9$r$r$&$^$/07$&$?$a$N4X?t(B
    static int goodCHptr(void* t) { return ((((uint64_t) t) & 1) == 0); }
    static void* index_to_CHptr(int i) { return (void*)((i << 1) + 1); }
    static int CHptr_to_index(void* t) { return ( ((unsigned) t) >> 1); }
};

inline int ptrHash(const Pix& key) { return ((int64_t)key / sizeof(Pix)); }
inline int intHash(const int& key) { return key; }

#include "Cmap.cpp"

#endif
