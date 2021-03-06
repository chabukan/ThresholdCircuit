#ifndef _IsvList_H
#define _IsvList_H

/*
  IsvList<T>: intrusive ($B?/F~E*(B) doubly-linked list

  $BMWAG(B T $B$r$=$N$^$^%j%9%H$K7R$0!%(B
  $B$=$N$?$a!$MWAG(B T $B$O!$(BILNode $B$r7Q>5$7$J$1$l$P$J$i$J$$!%(B
  $B%j%9%H$N30B&$G(B new/delete $B$r$9$kI,MW$,$"$k!%(B
  $BC"$7!$(BIsvList$B$r(Bdelete$B$9$k$H$-$K;D$C$F$$$kMWAG$O!$<+F0E*$K(Bdelete$B$5$l$k!%(B
 */

// todo: cube$B$N$_;H$o$l$F$$$k(B

struct ILNode
{
    ILNode*	fw; // center: $B%j%9%H$h$j<!$N%N!<%I$N%]%$%s%?(B
    ILNode*	bw; // center: $B%j%9%H$h$jA0$N%N!<%I$N%]%$%s%?(B

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

    void	append(ILNode* item); // $B:G8e$KA^F~(B
    void	insertAfter(ILNode* item, ILNode* before); // before$B$N8e$KA^F~(B

    void	prepend(ILNode* item); // $B:G=i$KA^F~(B
    void	insertBefore(ILNode* item, ILNode* after); // after$B$NA0$KA^F~(B

    ILNode*	remove(ILNode*& ptr);
    	// ptr$B$K;X$5$l$?$b$N$r%j%9%H$+$i:o=|$7$FJV$7!$(Bptr$B$O8e$N(Bobject$B$r;X$9!%(B

    // for ( ILNode* ptr = a.first(); ptr; a.next(ptr) ) { use ptr; }
    ILNode*	first() const { return head; }
    void	next(ILNode*& ptr) const { ptr = ptr->fw; }

private:
    ILNode*	head; // center: $B:GA0Ns(B
    ILNode*	tail; // center: $B:G8eHx(B
    int		n_elem; // center: $B%5%$%:(B
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

    void	append(T* item) { rep.append(item); } // $B:G8e$KA^F~(B
    void	insertAfter(T* item, T* before) // before$B$N8e$KA^F~(B
    		{ rep.insertAfter(item, before); }

    void	prepend(T* item) { rep.prepend(item); } // $B:G=i$KA^F~(B
    void	insertBefore(T* item, T* after) // after$B$NA0$KA^F~(B
    		{ rep.insertBefore(item, after); }

    T*		remove(T*& ptr);
    	// ptr$B$K;X$5$l$?$b$N$r%j%9%H$+$i:o=|$7$FJV$7!$(Bptr$B$O8e$N(Bobject$B$r;X$9!%(B

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
