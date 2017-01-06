#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib>


//これは、ISF の　内部的に使うもの　（古い実現で新しい実現の方をこれから使おう(06/31)
//＊＊＊＊＊＊＊＊＊ 非冗長積和形の表現するためのクラス
// 非冗長 積和形  Vi(f1) + ~Vi(f0) + fr 
//**********************************
// ＊＊＊（注意事項)
/*

print_for_debugで表示できる。

ISOP              i         f0                   f1               fr
Bdd::var(3)      2        BDDF                 BDDT              BDDF
BDDF            -2          0                    0                0
BDDT            -1          0                    0                0

組み合わせの場合

ISOP = x4 ~x2 なら

ISOP           3          ISOP0                ISOP1           ISOPR
ISOP0         -2            0                    0               0   -> 要注意
ISOPR         -2            0                    0               0
ISOP1          2          ISOP10               ISOP11           ISOP1R

ISOP10        -2            0                    0               0
ISOP11        -2            0                    0               0   
ISOP1R         1         ISOP110               ISOP111           ISOP11R 
                         ( i= -1)              ( i= -2)          ( i= -2) 
                                    --> ない場合は -2 

★ 必ずしも i は 4, 3, 2, 1 と減っていくとは限らない(抜けてる場合あり)
 -> makeISOPはtopの変数で展開してそれを、i としてるので。
  たまに依存してない部分があれば i の番号はは抜けになる
 // ただし、 f0, f1 が ともに BDDF をさすような ISOP は 作らずにその場合は、
 //  そのISOPの fr 自身を返すように makeISOPを変更した(1997/05/21)
 */

class ISOP{
private:
    short int    i;          // 変数番号 (0からなので、(BDDのlev -1) )  
    ISOP* f0;               // 変数番号 -1 は 恒真関数を表す。
                             // 変数番号 -2 は 恒偽関数を表す。
    ISOP* f1;               //f0, f1, frがない場合は = 0 (nil)のはず
    ISOP* fr;
//    ISOP(){std::cerr<<"ISOP Error\n";exit(1);}        // 引数なしはエラー
    ISOP(){}        // 引数なしはエラー
    static int objnum;
    int  TrueOrFalse();       // 恒真 ー １ 恒偽 ー 0 その他 2 を返す。
public:
  ISOP(int j);
  ~ISOP();
  ISOP * F0() { return f0;};
  ISOP * F1() { return f1;};
  ISOP * FR() { return fr;};
  short int Number(){return i;};
  void print(int flag);         // 表示 flag=0 は括弧非表示
  Bdd makeLF(); // ISOP の 表現する 論理関数を表すBddをつくって返す。
  void print_for_debug();   //debug の ため、
friend ISOP* ISF::makeISOP();
};


//int ISOP::objnum=0;

inline int ISOP::TrueOrFalse () {
    if(i == -1) return 1;
    else if(i == -2 ) return 0;
    else return 2;
}

