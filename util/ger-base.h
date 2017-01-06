#ifndef _ger_base_h
#define _ger_base_h

//#include <string.h>
//#include <string>
//#include <stdio.h>
//#include <stdlib.h>
//#include <fstream.h>

#include "bddop.h"
#include "twobdd.h"
#include "Array.H"
#include "arrayint.h"
#include "sop.h"
#include "cube.h"

#include "SupVars.h"


//関数の表示や操作など libnetwork.a より先にリンクしないとだめなのもある。
#ifndef nil
#define nil 0
#endif


extern int      word_len(const char* c);
// スペースまでの文字数をかえす
// 1文字目は空白でないこと


//SupVars に含まれるリテラルすべてについてsmoothしたのを返す。
// ★ 1998 07 22 より smooth_cube_all に 取って代わった.
Bdd  smooth_all(Bdd& F,  SupVars & sup_vars);



//TwoBdd を 真理値表で表示 (8入力以下のみを扱う．)
//string の前から、2つずつの文字を、(bit1,2) = (00, 01, 10. 11)に対応させる
//例 string = " 0*0 1*1" 
//extern void Print_Karnaugh_Map(TwoBdd& twobits, FILE *sout, int n_in, char * string);
//   -> isf-ger.cc にとりあえず移動

extern void printArrayInt(Arrayint Array);


/// ZBDD 関係

const int Digit = 3; //zbddの表示時に、仮定している変数番号の最大桁数。x1-x999まで可能

// ZBDDの情報の表示 - 集合の要素など (for Debug)
extern void printZBDDinfo(Bdd& Zbdd);

// ZBDDの集合の要素を列挙して表示 
extern void printZBDD(FILE * sout, Bdd& Zbdd);


///// ★　関数の表示関係　(netview 等でつかってるものあり）

// SopのBddを和積形でstderr に 表示 ２項以上あれば()をつける。
// 恒真、恒偽関数では何もしない。
// flag = 0 の時は 括弧をつけない。
// 最初に呼ぶときは、flag = 0　で呼ぶ
extern void  sop_print(FILE * sout, Sop  sop, int flag);


// var2idx: Bddのvar(1から始まる)から，表示時のidx(0から始まる)への写像
//   (CubeSet::printSop と同じ）
// var2idx には、　net->getLevel2indexFrom0().getPtr() を呼べばいいはず
//  
extern void BddPrintPLA(FILE * sout, const Bdd& lf, int* var2idx=0);

extern void Bdd_printTable_normal(FILE *sout, int n_in, const Bdd F);
//***************
//n_in はBdd全体の変数の数を仮定している。つまり依存していない変数も考慮にいれている。
//★n_inにそれ以外の数を渡しても意味のある結果を表示することにはならない。
//ここで表示される　xi の番号は 1からで、 Bddの変数番号(lev)と一致
//***************

extern void Bdd_printTable_gray(FILE *sout, int n_in, const Bdd F);
//***************
//printTable_normal のグレイコードで表示するもの
//***************

// lf を非冗長積和形で表示 (ISOPは新しい実現に変えた　（０７／０１）
extern void printISOP(Bdd lf);
// f0 + f1 + fr の順で表示

extern void Bdd_print_SOP(FILE *sout, Bdd F);
//***************  展開順は、 rstr1, rstr0の順(
//SOP(積和形)として関数を表示
// 展開はBdd のTopの変数から順に行う。
// 不必要な括弧をつけないようにしている
// 展開時にわかる冗長な表現はしてないが、全体としては冗長な表現であることが多い。
//ここで表示される　xi の番号は 1からで、 Bddの変数番号(lev)と一致
// x4x3~x2x1+~x4x3x2 と表示されれば、x2 と　x4が否定という意味
//***************


extern void Array_Bdd_print(FILE *sout, Array<Bdd> Bdd_Array, int n_in);
//**注意 Bdd::null が Arrayに含まれてたら、落ちる。（エラーチェックなし）
// 複数のBdd （Array<Bdd)を横に並べて表示 --> 比較しやすい
//変数の数が9より多いと表がずれるので表示しない。
// Bdd_printTable_gray(FILE *sout, int n_in, const Bdd F)を同時に行う感じ


extern void Bdd_Karnaugh_Map(FILE *sout, int n_in, const Bdd F);
//***************
//カルノマップ表示を行う。上部には変数番号の多い方、
//左側には変数番号の少ない方(x1から）を表示する。
//変数の数が9より多いと表がずれるので表示しない。
//***************

// char s  は 呼び出し側で確保されていること
extern void itoa(int n, char *s);

#endif



 
