#ifndef _IsvList_H
#define _IsvList_H

/*
  IsvList<T>: intrusive (侵入的) doubly-linked list

  要素 T をそのままリストに繋ぐ．
  そのため，要素 T は，ILNode を継承しなければならない．
  リストの外側で new/delete をする必要がある．
  但し，IsvListをdeleteするときに残っている要素は，自動的にdeleteされる．
 */

// todo: cubeのみ使われている

struct ILNode
{
    ILNode*	fw; // center: リストより次のノードのポインタ
    ILNode*	bw; // center: リストより前のノードのポインタ

    ILNode(ILNode* f=0, ILNode* b=0) : fw(f), bw(b) {}
    virtual ~ILNode() {}
};

class IsvListBase {
public:
    IsvListBase() : head(0), tail(0), n_elem(0) {}
    IsvListBase(const IsvListBase& orig);
    ~IsvListBase() { clear(); }

    int         size() const { return n_elem; }
    void	clear();

    void	append(ILNode* item); // 最後に挿入
    void	insertAfter(ILNode* item, ILNode* before); // beforeの後に挿入

    void	prepend(ILNode* item); // 最初に挿入
    void	insertBefore(ILNode* item, ILNode* after); // afterの前に挿入

    ILNode*	remove(ILNode*& ptr);
    	// ptrに指されたものをリストから削除して返し，ptrは後のobjectを指す．

    // for ( ILNode* ptr = a.first(); ptr; a.next(ptr) ) { use ptr; }
    ILNode*	first() const { return head; }
    void	next(ILNode*& ptr) const { ptr = ptr->fw; }

private:
    ILNode*	head; // center: 最前列
    ILNode*	tail; // center: 最後尾
    int		n_elem; // center: サイズ
};

template <class T>
class IsvList {
private:
    IsvListBase rep;
public:
    IsvList() {}
    IsvList(const IsvList<T>& orig) : rep(orig.rep) {}
    ~IsvList() { clear(); }

    int         size() const { return rep.size(); }
    void	clear() { rep.clear(); }

    void	append(T* item) { rep.append(item); } // 最後に挿入
    void	insertAfter(T* item, T* before) // beforeの後に挿入
    		{ rep.insertAfter(item, before); }

    void	prepend(T* item) { rep.prepend(item); } // 最初に挿入
    void	insertBefore(T* item, T* after) // afterの前に挿入
    		{ rep.insertBefore(item, after); }

    T*		remove(T*& ptr);
    	// ptrに指されたものをリストから削除して返し，ptrは後のobjectを指す．

    // for ( T* ptr = a.first(); ptr; a.next(ptr) ) { use ptr; }
    T*		first() const { return (T*)rep.first(); }
    void	next(T*& ptr) const;
};

template <class T>
inline T* IsvList<T>::remove(T*& ptr) {
    ILNode* p = ptr;
    T* result = (T*)rep.remove(p);
    ptr = (T*)p;
    return result;
}

template <class T>
void IsvList<T>::next(T*& ptr) const {
    ILNode* p = ptr;
    rep.next(p);
    ptr = (T*)p;
}

#endif /* _IsvList_H */
