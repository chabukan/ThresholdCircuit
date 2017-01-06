#include "Cmap.H"
#include "stdio.h"

template <class K, class V>
Cmap<K,V>::Cmap(V dflt, int (*hashf)(const K& key), uint sz)
: tab_size(sz), def(dflt), hashfunc(hashf)
{
    assert(tab_size > 0);
    tab = new KVNode<K,V>*[tab_size];
    for (uint i = 0; i < tab_size; ++i)
	tab[i] = (KVNode<K,V>*) index_to_CHptr(i+1);
    count = 0;
}

template <class K, class V>
Cmap<K,V>::Cmap(const Cmap& a)
: def(a.def), hashfunc(a.hashfunc)
{
    tab = new KVNode<K,V>*[tab_size = a.tab_size];
    for (uint i = 0; i < tab_size; ++i)
	tab[i] = (KVNode<K,V>*) index_to_CHptr(i+1);
    count = 0;
    for (Pix p = a.first(); p; a.next(p)) (*this)[a.key(p)] = a.value(p);
}

template <class K, class V>
Cmap<K,V>& Cmap<K,V>::operator = (const Cmap& a)
{
    clear();
    for (Pix p = a.first(); p; a.next(p)) (*this)[a.key(p)] = a.value(p);
    return (*this);
}

template <class K, class V>
V& Cmap<K,V>::operator [](const K& item)
{
//     if ( crowded() ) resize();
    // やみくもにresize()するのはやめた．
    // これにより，以前の要素にアクセスしている限り，next()で繰り返えせる
    // 新たな要素にアクセスしようとしresize()が起こると，next()で繰り返せない

    uint h = (*hashfunc)(item) % tab_size;

    KVNode<K,V>* t;
    for (t = tab[h]; goodCHptr(t); t = t->tl)
	if ( item == t->hd ) return t->cont;

    // 以前に要素があれば，それを返して終り
    // なけれは，resize()が必要かどうかを調べる．
    if ( crowded() ) {
	resize();
	h = (*hashfunc)(item) % tab_size; // keyの再計算
    }

    t = new KVNode<K,V>(item, def, tab[h]);
    tab[h] = t;
    ++count;
    return t->cont;
}

template <class K, class V>
V Cmap<K,V>::get(const K& item) const
{
    uint h = (*hashfunc)(item) % tab_size;

    KVNode<K,V>* t;
    for (t = tab[h]; goodCHptr(t); t = t->tl)
	if ( item == t->hd ) return t->cont;

    return def;
}

template <class K, class V>
Pix Cmap<K,V>::first() const
{
    for (uint i = 0; i < tab_size; ++i)
	if (goodCHptr(tab[i])) return Pix(tab[i]);
    return 0;
}

template <class K, class V>
void Cmap<K,V>::next(Pix& p) const
{
    KVNode<K,V>* t = ((KVNode<K,V>*)p)->tl;
    if (goodCHptr(t)) p = Pix(t);
    else {
	for (uint i = CHptr_to_index(t); i < tab_size; ++i) {
	    if (goodCHptr(tab[i])) {
		p =  Pix(tab[i]);
		return;
	    }
	}
	p = 0;
    }
}

template <class K, class V>
Pix Cmap<K,V>::seek(const K& key) const
{
    uint h = (*hashfunc)(key) % tab_size;

    for (KVNode<K,V>* t = tab[h]; goodCHptr(t); t = t->tl)
	if ( key == t->hd ) return Pix(t);

    return 0;
}

template <class K, class V>
void Cmap<K,V>::del(const K& key)
{
    uint h = (*hashfunc)(key) % tab_size;

    KVNode<K,V>* t = tab[h]; 
    KVNode<K,V>* trail = t;
    while (goodCHptr(t)) {
	if ( key == t->hd ) {
	    if (trail == t) tab[h] = t->tl;
	    else trail->tl = t->tl;
	    delete t;
	    --count;
	    return;
	}
	trail = t;
	t = t->tl;
    }
    fprintf(stderr, "Cmap<K,V>::del(const K& key), no such item\n");
    assert(0);
}

template <class K, class V>
void Cmap<K,V>::clear()
{
    for ( uint i = 0; i < tab_size; ++i ) {
	KVNode<K,V>* p = tab[i];
	tab[i] = (KVNode<K,V>*) index_to_CHptr(i+1);
	while (goodCHptr(p)) {
	    KVNode<K,V>* nxt = p->tl;
	    delete p;
	    p = nxt;
	}
    }
    count = 0;
}

template <class K, class V>
void Cmap<K,V>::resize()
{
    int oldsize = tab_size;
    tab_size = tab_size<<1;

    KVNode<K,V>** oldtab = tab;
    tab = new KVNode<K,V>*[tab_size];

    for ( uint i = 0; i < tab_size; ++i )
	tab[i] = (KVNode<K,V>*) index_to_CHptr(i+1);

    for ( int i = 0; i < oldsize; ++i ) {
	KVNode<K,V>* p = oldtab[i];
	while (goodCHptr(p)) {
	    KVNode<K,V>* nxt = p->tl;

	    uint h = (*hashfunc)(p->hd) % tab_size;
	    p->tl = tab[h];
	    tab[h] = p;

	    p = nxt;
	}
    }
    delete [] oldtab;
}
