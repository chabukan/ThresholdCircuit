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
    Cmap(V dflt, int (*hashf)(const K& key), uint sz=INITCAPA); // マップ生成 -> コンストラクタ 
    Cmap(const Cmap& a); // aというマップをコピー -> コンストラクタ
    ~Cmap() { clear(); delete [] tab; } // 削除 -> clear()
    
    int		size() const { return count; }// current number of items //マップの要素数を返す -> size()
    void	clear(); // clear()
    void	resize();// サイズを２倍に拡張する -> 不必要？

    Cmap&       operator = (const Cmap& a); // 代入 -> コンストラクタ
    V&		operator [] (const K& key); // 無いときは，defへのmapを作る // そのkeyが管理する値を返す -> find()
    V		get(const K& key) const;    // 無いときは，defのコピーを返す．// そのkeyが管理する値を返す -> find()
    // v.find(key)->second or v[key]
    Pix		seek(const K& key) const; // そのkeyが管理するアドレスを返す -> equal_range()
    // m.count(n) == ?/ で代用
    void	del(const K& key); // 

    Pix		first() const; // Cmapの先頭を指すアドレスを返す -> begin()
    void	next(Pix& i) const; // 今指しているアドレスの次のアドレスを返す -> itrator++
    K&		key(Pix p) const // 今指しているアドレスのkeyを返す -> iterator.first()
    { assert( p != 0 ); return ((KVNode<K,V>*)p)->hd; }
    V&		value(Pix p) const // 今指しているアドレスのvalueを返す -> iterator.second()
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

    // Cmapのアドレスををうまく扱うための関数
    static int goodCHptr(void* t) { return ((((uint64_t) t) & 1) == 0); }
    static void* index_to_CHptr(int i) { return (void*)((i << 1) + 1); }
    static int CHptr_to_index(void* t) { return ( ((unsigned) t) >> 1); }
};

inline int ptrHash(const Pix& key) { return ((int64_t)key / sizeof(Pix)); }
inline int intHash(const int& key) { return key; }

#include "Cmap.cpp"

#endif
