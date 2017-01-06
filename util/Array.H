#ifndef _Array_h
#define _Array_h

//#include "stdlib.h"
#include "assert.h"

//center: networkやbdd/isfなどで使われている

template <class T>
class Array {
private:
    T*		ent; // 配列
    int		sizerep; // 配列の要素数
public:
    Array() : ent(0), sizerep(0) {} // 要素数0の配列を作成 -> コンストラクタ
    Array(const Array<T>& obj); // 配列objを元にして配列を作成 -> コンストラクタ
    Array(int n_elem); // n_elem文の要素を確保して配列を作成 -> コンストラクタ 
    ~Array() { if ( ent ) delete [] ent; } // 配列を削除 -> clear()?

    // 配列を削除して、要素数0の配列を作成 -> clear()
    void	giveContain(Array<T>& obj) {
        obj.clear();
        obj.ent = ent;		ent = 0;
        obj.sizerep = sizerep;	sizerep = 0;
    }
    T&		operator [] (int index) const; // index番目の要素を返す -> at(),*iterator
    Array<T>&	operator = (const Array<T>& obj);// 代入 -> =
    int		operator == (const Array<T>& obj); // 等価かどうかを評価 -> ==

    int 	size() const { return sizerep; } // 配列のサイズを返す -> size()
    T*	 	getPtr() { return ent; } // 配列の先頭のアドレス  -> begin()
    void	clear() { sizerep = 0; delete [] ent; ent = 0; } // 配列の削除 -> clear()
    // clear() + shrink_to_fit() (c+11)
    // or vector<T>().swap(v)
    // ※shrink_to_fit = vector<T>(v).swap(v);
    void	add(const T& adding); // 配列の末尾に要素を追加 -> push_back()
    void	add_a(const Array<T>& obj);// 配列の末尾に別の配列を追加 -> copyを使う
    // require #include <algorithm, iterator>
    int	del(const T& deling);// delingの要素を削除 -> ar.erase(find(ar.begin(), ar.end(), deling));
    void	del_i(int index); // 配列のi番目の要素を削除+詰める -> erase()
    void	replace(const T& deling, const T& adding); // 配列内すべてのdelingをaddingに置き換える -> find()をループ removeでも可能? http://ppp-lab.sakura.ne.jp/cpp/library/021.html
    // replace(v.begin(), v.end(), deling, adding)
    void	reserve(int newsize); // newsizeで配列を作りなおす -> resize(n)
    int		position(const T& item); // itemが入っているindexを返す -> find()
    int		contain(const T& item) { return (position(item) == -1) ? 0 : 1; } // 含まれている場合 1をリターン
    // find(ar.begin(), ar.end(), target) != ar.end() -> containのリターンが1の場合
    void	sort(int (*cmp)(const void*, const void*)); // ソートをする -> sort()
    void	fill(const T& value); // valueをすべての配列に与える -> assign()
};

#include "Array.cpp"

#endif /* _Array_h */
