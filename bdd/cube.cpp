#include "cube.h"
#include "arrayint.h"
#include "isf.h"
#include "sop.h"

int Cube::operator == (const Cube& obj)
{
    if ( size != obj.size ) return 0;
    else if ( strcmp(rep, obj.rep) != 0 ) return 0;
    else return 1;
}

void Cube::rstr(int varIdx)	// BDDF: varIdx, BDDT: ~varIdx
{
    int value = ( varIdx < 0 ) ? 1 : 0;
    int idx = value ? ~varIdx : varIdx;

    if ( rep[idx] == '0' ) {
        if ( value == 0 ) rep[idx] = '-';
        else if ( value == 1 ) size = 0;
    }
    else if ( rep[idx] == '1' ) {
        if ( value == 0 ) size = 0;
        else if ( value == 1 ) rep[idx] = '-';
    }
}

void Cube::jumpup(int i, int j)
{
    int in_i = rep[i];
    for ( int k = i; k < j; k++ ) rep[k] = rep[k+1];
    rep[j] = in_i;
}

Bdd Cube::getBdd(Arrayint& lev2idx)
{
    Bdd result = Bdd::one;

    for ( int lev = lev2idx.size(); --lev > 0; ) {
        int idx = lev2idx[ lev ];
        if ( idx < 0 || idx >= size ) continue;

        if ( rep[idx] == '0' ) result = result & ~Bdd::var(lev);
        else if ( rep[idx] == '1' ) result = result & Bdd::var(lev);
    }
    return result;
}

void Cube::print(FILE* sout, char* postString, int* permutation)
{
    if ( permutation ) {
        /* newline に permutation した後の cube 表現を作る．
         permutation の内容を見て，必要最小限の長さにする */
        int maxp = -1;
        for ( int i=size; --i>=0; ) {
            int p = permutation[i];
            if ( p < 0 ) continue;
            if ( p > maxp ) maxp = p;
        }

        char* newline = new char[maxp+2];
        newline[maxp+1] = '\0';
        for ( int i=maxp; --i>=0; ) newline[i] = '-';

        for ( int i=size; --i>=0; ) {
            int p = permutation[i];
            if ( p < 0 ) continue;
            newline[p] = rep[i];
        }

        fprintf(sout, "%s%s\n", newline, postString);
        delete [] newline;
    }
    else {
        /* permutation がなければ，単に表示 */
        fprintf(sout, "%s%s\n", rep, postString);
    }
}

////////////////////

void CubeSet::compress()
{
    int i, j;
    int n = nIn();
    Arrayint depend( n );
    for ( i = n; --i>=0; ) depend[i] = 0;

    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        for ( i = n; --i>=0; )
            if ( c->value(i) == '0' || c->value(i) == '1' ) depend[i] = 1;
    }

    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        for ( i=0, j=0; i<n; i++ ) {
            if ( depend[i] != 0 ) c->setValue( j++, c->value(i) );
        }
    }

    numIn = j;

    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        c->setValue( numIn, '\0' );
    }
}

void CubeSet::rstr(int varIdx)	// varIdx: 1そのまま 0否定~
{
    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        c->rstr(varIdx);
    }	

    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        if ( c->empty() ) {
            Cube* delc = f1.remove(c);
            delete delc;
        }
    }
}

void CubeSet::print(FILE* sout, char* postString, int* permutation)
{
    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        c->print(sout, postString, permutation);
    }
}

void CubeSet::jumpup(int i, int j) // インデックスiをjにまでjumpup
{
    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        c->jumpup(i,j);
    }
}

Bdd CubeSet::getBdd(Arrayint& lev2idx)
{
    Bdd result = Bdd::zero;
    for ( Cube* c = f1.first(); c; f1.next(c) ) {
        result = result | c->getBdd(lev2idx);
    }
    return result;
}

// このCubeSetのコンストラクタとその次のmakeCubeSopはペアになっている．
// これらの中で Cube() を new しているところが3箇所あるが，
// それらの Cube() は，makeCubeSopの終端ケースで，
// f1.appendでf1に付けられるか，さもなければdelete される．

CubeSet::CubeSet(int nIn, const Sop& origSop) : numIn(nIn)
{
    Cube* nowCube = new Cube(nIn);
    makeCubeSop(nIn, origSop, nowCube, 0);
}

void CubeSet::makeCubeSop(int nin, const Sop& f, Cube* nowCube, int nowno)
{
    int check = nin - nowno;  // この番号の配列まで、ちゃんとはいってる。

    if ( f == Sop::one ) {// この時のみ付け加える．NowCubeに今までのが設定．
        for ( int i=0; i<check; i++ ) //これより下は、NowCubeが設定されてない。
            nowCube->setValue(i, '-');
        f1.append( nowCube );
        return;
    }
    else if ( f == Sop::zero ) {
        delete nowCube;
        return; // この時は付け加えずにreturn
    }

    // 2つコピーして3つにする．
    Cube* newCube0 = new Cube( *nowCube );
    Cube* newCube1 = new Cube( *nowCube );
    Cube* newCubeR = nowCube;

    // Cubeの添字は0からn-1
    // varは1からn
    int var = f.topvar();

    //ISOPの変数番号が飛んでることもあるので
    for ( int i = var; i<check; i++ ) {
        newCube0->setValue(i, '-');
        newCube1->setValue(i, '-');
        newCubeR->setValue(i, '-'); 
        nowno++;
    }

    newCube0->setValue(var-1, '0');
    newCube1->setValue(var-1, '1');
    newCubeR->setValue(var-1, '-');

    makeCubeSop(nin, f.factor0(var), newCube0, nowno+1);
    makeCubeSop(nin, f.factor1(var), newCube1, nowno+1);
    makeCubeSop(nin, f.factorR(var), newCubeR, nowno+1);
}
    
void CubeSet::printSop(Sop f, FILE* sout,
                       char* postString, int* var2idx)
{    // var2idx: Sopのvar(1から始まる)から，表示時のidx(0から始まる)への写像
    if ( f == Sop::null ) return;
    int dummyN = f.topvar();
    CubeSet cs = CubeSet(dummyN, f); // 添字: Sopは1から，CubeSetは0から
    if ( var2idx ) {
        cs.print(sout, postString, var2idx+1);
    	// CubeSetのprintの引数であるpermutationに変換するため．
    	// 結果的にvarを1ずつ減らすことになる．
    }
    else {
        cs.print(sout, postString, 0);
    }
}

void CubeSet::printSop(const Bdd& f, FILE* sout, int* var2idx)
{
    if ( f == Bdd::one ) {
        fprintf(sout, "1\n");
        return;
    }
    else if ( f == Bdd::null || f == Bdd::zero ) return;

    Sop isop = IsfLU(f).makeISOP();

    Sop NOTisop = IsfLU(~f).makeISOP(); 

    if( NOTisop.nCube() < isop.nCube())
        CubeSet::printSop(NOTisop, sout, (char*)" 0", var2idx);
    else
        CubeSet::printSop(isop, sout, (char*)" 1", var2idx);
}
