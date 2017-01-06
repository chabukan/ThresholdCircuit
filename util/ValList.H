#ifndef _ValList_H
#define _ValList_H

/*
  ValList<T>: 値をコピーして格納する doubly-linked list

  iteratorとして，Pixを用いる．
  要素 T への参照は value(p) で得られる．
 */

typedef void* Pix;

template <class T>
struct VLNode
{
    T		obj;
    VLNode<T>*	fw;
    VLNode<T>*	bw;

    VLNode(const T& someobj, VLNode<T>* f=0, VLNode<T>* b=0) 
    : obj(someobj), fw(f), bw(b) {}
};

template <class T>
class ValList {
public:
    ValList() : head(0), tail(0), n_elem(0) {}
    ValList(const ValList<T>& orig);
    ~ValList() { clear(); }

    int         size() const { return n_elem; }
    void	clear();

    Pix 	append(const T& item); // 最後に挿入
    // v.push_back(item)
    Pix 	insertAfter(const T& item, const T& before);
    		// beforeの後に挿入， beforeは後ろから探す．
    // v.insert(++find(v.begin(), v.end(), before), item) todo: 検証

    Pix 	prepend(const T& item); // 最初に挿入
    // v.push_front(item)
    Pix 	insertBefore(const T& item, const T& after);
    		// afterの前に挿入，afterは前から探す．
    // v.insert(find(v.begin(), v.end(), after), item)

    void	remove(const T& item); // itemを探して削除
    void	remove(Pix& p);
    		// pに指されたobjectを削除して，pは後のobjectを指す．
    // v.erase(find(v.begin(), v.end(), item))
    // 同じ要素が2つ以上ないならば v.remove(item)

    // for ( Pix p = a.first(); p; a.next(p) ) { use value(p); }
    Pix		first() const { return head; }
    // v.begin()
    void	next(Pix& p) const { p = ((VLNode<T>*)p)->fw; }
    T&		value(Pix p) const { return ((VLNode<T>*)p)->obj; }


private:
    VLNode<T>*	head;
    VLNode<T>*	tail;
    int		n_elem;

    VLNode<T>*	search_fw(const T& item);
    VLNode<T>*	search_bw(const T& item);
};

#include "ValList.cpp"

#endif /* _ValList_H */
