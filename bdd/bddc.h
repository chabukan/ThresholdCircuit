#ifndef _bddc_h
#define _bddc_h

#include <stdio.h>

typedef int bddid;
typedef unsigned int uint;

//// 共通

extern const int BDDNULL; // null
extern const int MAXLEV; // 2^14 -1 = 0011..1 == 16384-1

extern void     bddalloc(int init_p, int max_p); // 初期化
extern void     bddstatus(); // 演算結果テーブルの使用率を表示
extern void     bddgc(); // ガーベッジコレクション
extern int      bddused(); // 使用ノード数

extern bddid    bddcopy(bddid f); // コピー
extern bddid    bddfree(bddid f); // 開放

extern bddid    bddvar(int lev); // 変数x_levを返す, 1 <= lev <= MAXLEV

extern bddid    bddvarset(bddid f); // 依存変数

extern int      bddtop(int f); // 最上位の変数のレベル
extern int      bddsize(bddid f, int i, int j); // ノード数
extern int      bddsize(bddid f, int lev); // ノード数
extern int      bddsize(bddid f); // ノード数

extern bddid    bddreadcache(int op, bddid f, bddid g); // 演算テーブル読み
extern void     bddwritecache(int op, bddid f, bddid g, bddid h); //   書き

//// BDD

extern const int BDDF; // false, 真理値0
extern const int BDDT; // true, 真理値1

inline bddid    bddnot(bddid f) { return bddcopy(~f); } // NOT
extern bddid    bddand(bddid f, bddid g); // AND
inline bddid    bddor(bddid f, bddid g) { return ~bddand(~f, ~g); } // OR
extern bddid   	bddxor(bddid f, bddid g); // XOR

extern bddid    bddsmooth(bddid f, int lev); // f_{lev=0} + f_{lev=1}
inline bddid    bddconsensus(bddid f, int lev) { return ~bddsmooth(~f, lev); }

extern bddid   	bddrstr(bddid f, int lev); // lev: BDDF, ~lev: BDDTを代入
extern bddid    bddrstrtop0(bddid f); // 最上位の変数に代入
extern bddid    bddrstrtop1(bddid f); // 最上位の変数に代入

extern bddid	bddcompress(bddid f); // レベルを詰める
extern bddid   	bddlevshift(bddid f, int lev, int degree); // レベルをシフト
extern bddid   	bddjumpud(bddid f, int i, int j); // レベルを変更

extern bddid    bddcofact(bddid f, bddid g); // gをケアセットとしてfを簡単化
extern int      bddinter(bddid f, bddid g); // 交わりがあるかどうか
extern bddid    bddvarite(int lev, bddid f1, bddid f0);// x_lev f1 + ~x_lev f0
extern bddid    bddcompose(bddid f, int lev, bddid g); // f(x_lev = g)
extern uint     bdddense(bddid f); // 真理値表密度

extern void     bddprintbdl(bddid f, FILE* bdlout); // BDL形式で出力

//// ZBDD

extern const int ZBDDE; // empty  { }, 空集合
extern const int ZBDDB; // base {0..0}, 0..0のみを要素とする集合

extern bddid    zbddchange(bddid f, int lev); // fの全要素で,levの有無を反転
extern bddid    zbddsubset(bddid f, int lev); // lev: subset0, ~lev: subset1

extern bddid    zbddintersec(bddid f, bddid g); // 積集合
extern bddid    zbddunion(bddid f, bddid g); // 和集合
extern bddid    zbdddiff(bddid f, bddid g); // 差集合
extern bddid    zbddproduct(bddid f, bddid g); // 直積集合
extern bddid    zbdddivide(bddid f, bddid g);  // f / g :  algebraic-division

extern uint     zbddcard(bddid f); // 集合の要素数
extern uint     zbddcard(bddid f, int lev); // levを含む集合の要素数
extern int      zbddmulti(bddid f); // zbddcardの結果が0か1か2以上か
extern int      zbddmulti(bddid f, int lev); // zbddcardの結果が0か1か2以上か
extern int      zbddlit(bddid f); // 集合の全リテラル数
extern void     zbddlitcount(bddid f, uint* literals); // 各リテラルの出現数
extern void     zbddlitcount(bddid f, uint* literals, int max); // max以下のみ
extern void     zbddmultilit(bddid f, char* literals);

extern bddid    zbddcommonlit(bddid f);  // すべてのcubeに出てくるlitの和を返す
extern bddid    zbddsum2product(bddid f); // 和形を積形に, a + b + c -> abc
extern bddid    zbddDelLitCube(bddid f); // 1リテラルのcubeを除く

extern bddid    zbddsop2func(bddid f); // Sop表現が f である関数を返す

extern bddid    zbddjumpup(bddid f, int i, int j); // レベルを変更
extern bddid    zbddlevshift(bddid f, int lev, int degree); // レベルをシフト

#endif
