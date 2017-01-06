#ifndef _arrayint_h
#define _arrayint_h

#include <stdlib.h>
#include <assert.h>

// center: sopやbddなどで使われている

class Arrayint {
private:
    int*	ent; // sizerepが0の時でも，ポインタを持つ．1999.1.14
    // でも今は，やっぱり0かもしれない．
    // free(0)ってOK?
    int		sizerep;
public:
    Arrayint() : ent(0), sizerep(0) {}
    Arrayint(int newsize);
    //     	Arrayint(int newsize=0);
    Arrayint(const Arrayint& obj);
    Arrayint(int size, int* array);
    ~Arrayint() { free(ent); }

    int&	operator [] (int index) const;
    Arrayint&	operator = (const Arrayint& obj);
    int	operator == (const Arrayint& obj);

    int		size() const { return sizerep; }
    int*	getPtr() { return ent; } //todo:  v.data()で引数を渡す
    void	clear() { sizerep = 0; free(ent); ent = 0; }
    void	add(int adding); // = push_back
    void	add_a(const Arrayint& obj); //
    int		del(int deling);
    void	del_i(int index);
    void	replace(int deling, int adding);
    void	reserve(int newsize);
    int	position(int item);
    int	contain(int item) { return (position(item) == -1) ? 0 : 1; }
    void	fill(int value);
    // ここまでの関数はArray.hと同じ
    void	print();// ファイルにArrayの中身を出力（エラーの時に使用）


    // permutationとしてみたときの逆写像を求めて返す．自分自身は変化しない.
    Arrayint	inverse();// その配列のindexとそれに対応した値の関係が逆転した配列を返す -> 

    // 以下はすべて自分自身を変更
    void	sort(int (*cmp)(const void* a, const void* b));// Arrayにも存在 -> sort()
    void	jumpup(int i, int j);// iに入っている値をjへ移動させる(i < j) -> erase(),insert()
    void	jumpdown(int i, int j);// iに入っている値をjへ移動させる(i > j) -> erase(),insert()
    void	jumpud(int i, int j);// i,jの値によって、jumpupとjumpdownに振り分ける
    // util.hのjumpud(v, i, j)
    void	shift(int index, int degree, int defv);// indexからdegree分だけずらす(degree > 0なら右にずらす、空いたところはdefv)(degree < 0なら左にずらす、ずれる前の値は消える) -> insert()
    //     Arrayint&	changeOrder(const Arrayint& permutation);
    void	changeOrder(const Arrayint& permutation);// indexとその中の値が同じになるようにソート -> sort()?
};

#endif /* _arrayint_h */
