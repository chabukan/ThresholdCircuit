#ifndef _MaxHeap_H
#define _MaxHeap_H

#include "stdio.h"
#include "Cmap.H"

/*
  MaxHeap<T>: 格納するのはTへのポインタのみ．実体はコピーしない．
 */

typedef int ktype;

struct HeapElem
{
    HeapElem(int k, void* i) : key(k), info(i) {}

    ktype key;
    void* info;
};

class MaxHeapBase {
public:
    static const int INITS = 1<<3; // 8
    MaxHeapBase() : elements( new HeapElem*[INITS] ),
                array_size(INITS), n_elem(0), hash(0,ptrHash) {}
    ~MaxHeapBase();

    void	insert(ktype key, void* item); // 挿入してupheap
    void*	delmax(); // 最大値の削除
    ktype       maxkey(); // 最大値を得る
    void*       maxinfo(); // 最大値を持つinfo
    void        update(ktype newkey, void* item); // keyを更新
    void        del(void* item); // itemの削除
    ktype       itemkey(void* item); // itemの値
    int         exist(void* item); // itemがあるかどうか

    int         size() const { return n_elem; }
    void        fprint(FILE* fp) const;

    ktype       key(int i) { return elements[i]->key; }
    void*       info(int i) { return elements[i]->info; }

private:
    void        upheap(int idx);
    void        downheap(int idx);
    void        resize(int newsize);

    HeapElem**      elements;
    int             array_size;
    int		    n_elem;
    Cmap<void*,int> hash;
};

template <class T>
class MaxHeap {
private:
    MaxHeapBase rep;
public:
    MaxHeap() {}
    ~MaxHeap() {}

    void	insert(ktype key, T* item) { rep.insert(key, item); }
    T*	        delmax() { return (T*)rep.delmax(); } // 最大値の削除
    ktype       maxkey() { return rep.maxkey(); } // 最大値を得る
    T*          maxinfo() { return (T*)rep.maxinfo(); } // 最大値を持つinfo
    void        update(ktype newkey, T* item) {
	return rep.update(newkey, item); // keyを更新
    }
    void        del(T* item) { rep.del(item); } // itemの削除
    ktype       itemkey(void* item) { return rep.itemkey(item); } // itemの値
    int         exist(void* item) { return rep.exist(item); } // あるかどうか

    int         size() const { return rep.size(); }
    void        fprint(FILE* fp) const { rep.fprint(fp); }

    ktype       key(int i) { return rep.key(i); }
    T*          info(int i) { return (T*)rep.info(i); }
};


#endif /* _MaxHeap_H */
