#ifndef __GENERAL_H__
#define __GENERAL_H__

#include "isf.h"
#include "ger-base.h"

/* node.h */

enum node_types {
    FREED,         // delete されたことを示す
    INPUT, OUTPUT, // 入力，出力
    LUT,           // funcを基本として内部論理を表現
    SOP,           // funcではなくcsetを基本として内部論理を表現
    AND, XOR, SYM, // andゲート，xorゲート，対称関数
    CONSTANT,      // 定数 0 or 1
    UNDEFINED,      // 未定義

    TH             // しきい値ゲート
};
// NOT ゲートは，各ゲートの入力反転で表現することにした．1996.4.24

/* -- node.h -- */

//static void cubeOrderSub(Sop cSet, int* order);
//	1995.12.22
// Find a variable that appear in cSet (a set of cubes) the most time.
// Order the variable from the top to the bottom.
//
void cubeOrder(const Sop& cSet, vector<int>& lev2idx, int n);

#endif // #ifndef __GENERAL_H__
