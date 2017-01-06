#ifndef _Arrayw_h
#define _Arrayw_h

#include "arrayint.h"

/*****************************************
** Assertions for T are 		**
**	sizeof(T) == sizeof(int)	**
**	operator = of T is just copying **
*****************************************/


// center: networkやnodeなどで使われている
// すべてarrayintと変わりがない

template <class T>
class Arrayw {
private:
    Arrayint rep;
public:
//     	Arrayw() {}
    	Arrayw(int newsize=0) : rep(Arrayint(newsize)) {}
    	Arrayw(const Arrayw<T>& obj) : rep(obj.rep) {}
    	~Arrayw() {}

    T&		operator [] (int index) const { return (T&) rep[index]; }
    Arrayw<T>&	operator = (const Arrayw<T>& obj)
    	{ return (Arrayw<T>&) (rep = obj.rep); }
    int		operator == (const Arrayw<T>& obj) { return rep == obj.rep; }

    int         size() const { return rep.size(); }
    T*          getPtr() { return (T*)rep.getPtr(); }
    void	clear() { rep.clear(); }
    void	add(T adding) { rep.add((int)adding); }
    void	add_a(const Arrayw<T>& obj) { rep.add_a(obj.rep); }
    int		del(T deling) { return rep.del((int)deling); }
    void	del_i(int index) { rep.del_i(index); }
    void	replace(T deling, T adding)
	{ rep.replace((int)deling, (int)adding); }
    void	reserve(int newsize) { rep.reserve(newsize); }
    int		position(T item) { return rep.position((int)item); }
    int		contain(T item) { return rep.contain((int)item); }
    void	fill(T value) { rep.fill((int)value); }
    void	print() { rep.print(); }

    // $B0J2<$O$9$Y$F<+J,<+?H$rJQ99(B
    void	sort(int (*cmp)(const void* a, const void* b)) {
	rep.sort(cmp);
    }
    void	jumpup(int i, int j) { rep.jumpup(i,j); }
    void	jumpdown(int i, int j) { rep.jumpdown(i,j); }
    void	jumpud(int i, int j) { rep.jumpud(i,j); }
    void	shift(int index, int degree, T defv)
    	{ rep.shift(index, degree, (int)defv); }
//     Arrayw<T>&	changeOrder(const Arrayint& permutation)
//     	{ return (Arrayw<T>&) rep.changeOrder(permutation); }
    void	changeOrder(const Arrayint& permutation)
    	{ rep.changeOrder(permutation); }
};

#endif /* _Arrayw_h */
