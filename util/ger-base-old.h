
#ifndef _ger_base_old_h
#define _ger_base_old_h


//cubeBdd に含まれるリテラルすべてについてsmoothしたのを返す。
// cubeBdd は 正リテラルの積でないとだめ
Bdd  smooth_cube_all(Bdd F, Bdd cubeBdd);




//////////////// 不完全してい論理関数を２ビットのBddで表すくらす。

class ISF; 
class ISOP; 

/////// ★★ makeISOP関連を含め全てObsolete IsfLU 等をつかうべし．



static ISF      operator & (const ISF lhs, const ISF rhs);
static int      operator == (const ISF lhs, const ISF rhs);
static int      operator != (const ISF lhs, const ISF rhs);
static int      operator <= (const ISF lhs, const ISF rhs);


//    ISF のコーディング
// F           low   up
// 0 ------>    0     0
// 1 ------>    1     1
// * ----->     0     1  



class ISF{
private:
    Bdd       low;     // lower bound
    Bdd       up;      // upper bound
public:
//  Sop makeISOPnew(int* lev2idx=0);  -> ResultPair dft つかわんように
//                                       もう使うのやめた．
//      -> 実体は，MakeIsopHash.hに保存してある．


  Sop makeISOPnewNoHash(int* lev2idx=0);  //Hash なし
    inline ISF();
    inline ISF(const ISF& oprand);
    inline ISF(Bdd lf);
    inline ISF(Bdd f0, Bdd f1);
  inline Bdd Low(){return low;};
  inline Bdd Up(){return up;};
//    ~ISF();                   // 自動的にup,lowのデストラクターよぶはず
    ISF     operator = (const ISF oprand);
    ISF     operator ~ () const;
        //ISFの否定を作って返す 0->1, 1->0. *->* と変化させる。
    ISOP*  makeISOP();
    int    check();    // このISF が おかしかったら ０を返す。(それ以外1)
    ISF     rstr0(const int lev) const;
    ISF     rstr1(const int lev) const;
    int     AllTrue();                        // 恒真関数を含めば１
    int     AllFalse();
    ISF    makeNECK(int i);
    void   print(FILE *sout,int n_in);   //Truth Tableで         
    void   print();                      //積和形で
    Bdd    pf1() const;
    Bdd    pf0() const;
    Bdd    pfStar() const;
    Bdd    pfSpecified();

    int    IsSatisfy(Bdd lf);
      // lf が、このISFに含まれるなら 1,
      // 含まれないなら　0を返す。
      // 含まれるとは、low <= lf <= up を満たすということである。

    int    topVarId() const;
      // low up のグラフの最上位の変数ノードのグラフで上にある方の変数番号
      // （澤田さんならlev、geertならvarid)
    void   maskISF(Bdd* f);         //bdd のところを*にする
    void  specify(Bdd* lf, Bdd* specify);
    //specify = 1 と なる部分を lfでおきかえる。（＊を狭める）
friend ISF      operator & (const ISF lhs, const ISF rhs);
inline friend int       operator == (const ISF lhs, const ISF rhs);
inline friend int       operator != (const ISF lhs, const ISF rhs);
inline friend int        operator <= (const ISF lhs, const ISF rhs);
friend void printISOP(Bdd lf);
friend void supportMain(int sup, ISF *f,int* varTable, int Nsup, int elim, 
		 ISF* minf,int* minNsup,int* varResult, int* Usedflag);


friend ISF support_min(ISF* f,int sup,int* varTable,int* nsup, int* varResult);
// 不完全指定論理関数 f の support数最小の不完全指定論理関数を返す。
// sup   --- 最低でも関係している変数の数
// varTable -- 最低でも関係している変数番号（BDDの番号なので、１から）
// varTable[0]-varTable[sup-1] の配列
// nsup -- 最小のsupport数
// varResult[i] = 0 なら varTable[i] の変数は使用される。
// varResult[i] = 1 なら varTable[i] の変数は使用されない。
// この、０、１も設定する。
// varTable[sup-1] から 除けるかチェックしていくので、BDDの変数が昇順にこの
// 配列に入っている方が処理時間が早くなることが期待される


};


//#include "ISF.h"
#include "IsfGer.h"
#include "OldISOP.h"

////////////////////  // PList 関係   --- ポインタのリスト　
/*
template <class T>
class PList{
public:
    PList*	next;
    T*          body;

}
＊＊＊＊＊＊＊注意
あくまでも、ポインタのリストであるから、PListのデストラクタはbody自身は
消さない。それも消して欲しい場合は、先にbodyを消してから、PListを消さね
ばならない。（例えば、Lut->logicなど、~Lut()をみよ）

デストラクタ はclearがよばれる。そのため、リスト全てが消される。

コンストラクタ
  //		PList() : next(nil), body(nil){}; -> 邪悪なのでやめ
    		PList(PList<T>* n, T* b) : next(n), body(b){};
    		PList(T* b) : next(nil),body(b){};
                ~PList(){};

リストのdelete(ポインタのみ）。リストたどって、すべて消す。
body の 指すオブジェクト自体のdelete はしてない。
    void	clear();

ノードitemをリストの最初ににつけ、その新しいリストを返す。
    PList*    newPList(T* item);

 PList<T>* reverse()
 リストを入れ換えたものを返す。元のリストはちゃんとポアする。
 //ただし、thisはnilにはならず、元のまま。thisにこの関数の返り値を代入
しないときは、要注意。

  PList<T>*      PListRemove(T* item);
// 該当するitemを除去し、そのリストを返す。
// 該当するものがなければ、えらーを表示して、thisを返す。
＊＊＊ nil PList にたいしてはエラーを起こす。（エラー処理なし）


bool       includeKa(T* item);   // 含まれてたらtrue
// nil PList なら falseを返すだけだが、item =nil なら、エラーとなる。


   PList<T>*       apendPList(PList<T>*  apend); 
          //apend を最後にくっつける。これは、ポインタをくっつけてるだけ



    PList<T>*  apendLastIfNot(T* item);   //含まれていなかったら、最後につける。
	       //そしてそのリストを返す。
           aholist = aholist->apendLast(boke) のように使うべし。
      //(aholist = nil の場合もうまく行くようちゃんと呼び出し側で代入すべし)
*/

#include "PList.h"

//template class PList<Node>;

/*   makdISOP はもう obsolete -> IsfLU->makeISOP　つかうべし
#include "MakeIsopHash.h"

extern void ClearMakeIsopHash();
// makeISOPでつかっているHashTable の　entryを全て空にする。
// これをしないと、実際には必要ないのに、makeISOPの途中でつかわれhashされている
// Bdd が　freeされない。
*/

#endif
