#ifndef _ValList_H
#define _ValList_H

/*
  ValList<T>: $BCM$r%3%T!<$7$F3JG<$9$k(B doubly-linked list

  iterator$B$H$7$F!$(BPix$B$rMQ$$$k!%(B
  $BMWAG(B T $B$X$N;2>H$O(B value(p) $B$GF@$i$l$k!%(B
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

    Pix 	append(const T& item); // $B:G8e$KA^F~(B
    // v.push_back(item)
    Pix 	insertAfter(const T& item, const T& before);
    		// before$B$N8e$KA^F~!$(B before$B$O8e$m$+$iC5$9!%(B
    // v.insert(++find(v.begin(), v.end(), before), item) todo: $B8!>Z(B

    Pix 	prepend(const T& item); // $B:G=i$KA^F~(B
    // v.push_front(item)
    Pix 	insertBefore(const T& item, const T& after);
    		// after$B$NA0$KA^F~!$(Bafter$B$OA0$+$iC5$9!%(B
    // v.insert(find(v.begin(), v.end(), after), item)

    void	remove(const T& item); // item$B$rC5$7$F:o=|(B
    void	remove(Pix& p);
    		// p$B$K;X$5$l$?(Bobject$B$r:o=|$7$F!$(Bp$B$O8e$N(Bobject$B$r;X$9!%(B
    // v.erase(find(v.begin(), v.end(), item))
    // $BF1$8MWAG$,(B2$B$D0J>e$J$$$J$i$P(B v.remove(item)

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
