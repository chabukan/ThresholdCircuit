#include "bddnode.h"
#include "bddop.h"
#include "varsymm.h"

//////////////////////////////////////////////////////////////////
//	symmetry
//////////////////////////////////////////////////////////////////

static void	rec_sym_analy(bddid f, VarSymm** matrix, int prevlev);
static void	rec_sym_ij(bddid f, int i, int j);
static void	rec_sym_ijsub(bddid f, bddid g, int lev);
static void	rec_andExt(bddid f, VarSymm* symRel);

static VarSymm sym_result;
static int rtsize_power = 10;
static int rtsize = 1<<rtsize_power;

static struct Result { bddid f, g; } * rtable;

#define rthash(f, g) 	( ( (f)^((f)<<4)^(g) ) & (rtsize-1) )

inline int RTCMP(Result* p, bddid f, bddid g) {
    return (p->f == f) && (p->g == g);
}
inline void RTWRITE(Result* p, bddid f, bddid g) {
    p->f = f;   p->g = g;
}

//////////////////////////////////////////////////////////////////

void Bdd::symAnaly(short* pairVar, VarSymm* symRel) const
{
    int i, j, n = top();
    //------ matrix: 各変数同士の対称性
    VarSymm** matrix = new VarSymm*[n+1];
    for ( i=2; i<=n; i++ ) matrix[i] = new VarSymm[i];

    // トラバースしてわかるsym
    rec_sym_analy(edge, matrix, 0);
    clearMark(edge);

    // 値を入れてわかるsym
    for ( i = n; i >= 3; i-- ) {
        if ( matrix[i][i-1].is_symm() ) continue;
        for ( j = i-2; j>=1; j-- ) {
            if ( ! matrix[i][j].is_symm() ) continue;
            matrix[i][j] = sym_ij(i, j);
            if ( matrix[i][j].is_symm() ) break;
        }
    }
    //     for ( i=n; i>1; i-- ) {
    //      printf("[%2d] ", i);
    //      for ( j=n-1; j>=i; j-- ) printf("   ");
    //      for ( j=i-1; j>=1; j-- ) { printf(" "); matrix[i][j].print_sym(); }
    //      printf("\n");
    //     }

    //------ pairVar 各変数で対称な変数，自身以下での最大のもの
    for ( i=n; i>=2; i-- ) for ( j=i-1; j>0; j-- )
                               if ( matrix[i][j].is_symm() ) {
                                   pairVar[i] = j;
                                   symRel[i] = matrix[i][j];   // 自分の対称性: PSYM and/or NSYM
                                   break;
                               }
    pairVar[1] = 0;
    //     printf("  ");
    //     for ( i=n; i>=1; i--) printf("%3d", pairVar[i]);
    //     printf("\n");

    for ( i=2; i<=n; i++ ) delete [] matrix[i];
    delete [] matrix;
}

VarSymm Bdd::sym_ij(int i, int j) const
{
    rtable = (Result*)calloc(rtsize, sizeof(Result));
    //     printf("%d.sym_ij(%d, %d)\n", edge, i, j);
    sym_result = VarSymm();
    rec_sym_ij(edge, i, j);
    clearMark(edge);
    //     assert( checkMark(edge) == 0 );
    free(rtable);
    //// calloc と delete 毎回は何だかもったいない気がする．
    return sym_result;
}

VarSymm svsym_result;

static void rec_svsym_ij(bddid f, int i, int j);
static void rec_svsym_ijsub(bddid f0, bddid f1, int lev);

// sdd5.log
// VarSymm Bdd::svsym_ij(int i, int j) const
// {
//     svsym_result = VarSymm();
//     rec_svsym_ij(edge, i, j);
//     clearMark(edge);
//     return svsym_result;
// }

// // sdd6.log
VarSymm Bdd::svsym_ij(int i, int j) const
{
    // とりあえず論理演算で実装．効率が気になるようであれば専用処理で
    // でもこっちのほうが速いことが多い．演算結果テーブルの効果か．
    VarSymm result;
    Bdd f0 = rstr0(i);
    Bdd f1 = rstr1(i);
    Bdd f0sup = f0.varSet();
    Bdd f1sup = f1.varSet();
    if ( f0sup.subset1(j) == Bdd::base ) { // jに依存
        result.neg_skip0();
    }
    if ( f1sup.subset1(j) == Bdd::base ) { // jに依存
        result.neg_skip1();
    }
    return result;
}


void Bdd::andExt(VarSymm* symRel) const
{
    rec_andExt(edge, symRel);
    clearMark(edge);
}

//////////////////////////////////////////////////////////////////

//// check asymmetric between level i and j ( i >= j+2 )
//// check symmetric between level i and j ( i == j+1 )
//// by traversing each node once ( except bddvarset )
////
static void rec_sym_analy(bddid f, VarSymm** matrix, int prevlev)
{
    bddid f_ = rmN(f);
    if ( f_ == BDDF ) return;
    int i, j, sup, tmpsup;

    /*------ asym1: path type, ~trav(i) and sup(j) ------*/
    int flev = node[f_].lev;
    if ( prevlev > flev+1 ) {
        tmpsup = sup = bddvarset(f_);
        while ( tmpsup != ZBDDE ) {
            j = node[rmN(tmpsup)].lev;
            for ( i=prevlev-1; i>flev; i-- ) matrix[i][j].neg_sym();
            tmpsup = rstrtop(tmpsup, BDDF); // subset0
        }
        bddfree(sup);
    }
    if ( node[f_].mark == 1 ) return;
    if ( flev <= 1 ) return;

    /*------ asym2: node type, flev and ~sup(k) ------*/
    tmpsup = sup = bddvarset(f_);
    j = flev;
    do {
        i = j;
        tmpsup = rstrtop(tmpsup, BDDF); // subset0
        j = node[rmN(tmpsup)].lev;	//// j == 0 if tmpsup == BDDT
        for ( int k=i-1; k>j; k-- ) matrix[flev][k].neg_sym();
    } while ( tmpsup != ZBDDE );
    bddfree(sup);

    /*------ sym between flev and flev-1 ------*/
    bddid f0 = rstrtop(f, BDDF);
    bddid f1 = rstrtop(f, BDDT);
    VarSymm* ptr = &(matrix[flev][flev-1]);
    if ( ptr->is_psym() ) {
        bddid f01 = rstrtopup(f0, ~(flev-1));
        bddid f10 = rstrtopup(f1,   flev-1 );
        if ( f01 != f10 ) ptr->neg_psym();
    }
    if ( ptr->is_nsym() ) {
        bddid f00 = rstrtopup(f0,   flev-1 );
        bddid f11 = rstrtopup(f1, ~(flev-1));
        if ( f00 != f11 ) ptr->neg_nsym();
    }

    //// recursive
    rec_sym_analy(f0, matrix, flev);
    rec_sym_analy(f1, matrix, flev);
    node[f_].mark = 1;
}

static void rec_sym_ij(bddid f, int i, int j)
{
    //////// terminal cases ////////
    bddid f_ = rmN(f);
    if ( f_ == BDDF ) return;
    if ( node[f_].mark ) return;

    int flev = node[f_].lev;
    if ( flev < j ) return;

    //////// recursive computation ////////
    if ( flev > i ) {
        rec_sym_ij(node[f_].edge0, i, j);
        if ( sym_result.is_symm() ) {
            rec_sym_ij(node[f_].edge1, i, j);
        }
    }
    else if ( i == flev ) {
        bddid f0 = rstrtop(f_, BDDF);
        bddid f1 = rstrtop(f_, BDDT);
        rec_sym_ijsub(f0, f1, j);
    }
    else {
        //// j <= flev < i, iが依存していないとき，jも依存していなければよい
        //// これはもうpath typeで調べてあるのでしなくても良い．
    }
    node[f_].mark = 1;
    //     printf("rec_sym_ij(%d, %d, %d) = %d, flev=%d\n", f, i, j, result, flev);
}

static void rec_sym_ijsub(bddid f0, bddid f1, int lev)
{
    //////// terminal cases ////////
    if ( f0 == f1 ) {
        bddid sup = bddvarset(f0);	//// lev以下も調べるのでもったいない?
        bddid suprstr = zbddsubset(sup, lev); // subset0
        if ( sup != suprstr ) {//// bddrstr(f0,lev,0) != bddrstr(f0,lev,1)
            sym_result.neg_sym();
        }
        bddfree(sup); bddfree(suprstr);
        return;
    }

    bddid f0_ = rmN(f0);
    bddid f1_ = rmN(f1);
    int f0lev = node[f0_].lev;
    int f1lev = node[f1_].lev;
    if ( (f0lev <= lev) && (f1lev <= lev) ) {
        if ( rstrtopup(f0,~lev) != rstrtopup(f1,lev) )
            sym_result.neg_psym();
        if ( rstrtopup(f0,lev) != rstrtopup(f1,~lev) )
            sym_result.neg_nsym();
        return;
    }

    /*-------- read the result table --------*/
    int key = rthash(f0, f1);
    if ( RTCMP(rtable+key, f0, f1) ) return;

    //////// recursive computation ////////
    bddid f00=0, f01=0, f10=0, f11=0;
    if ( f0lev >= f1lev ) {
        f00 = node[f0_].edge0;	f01 = node[f0_].edge1;
        if ( isN(f0) ) { f00 = ~f00; f01 = ~f01; }
    }
    if ( f1lev >= f0lev ) {
        f10 = node[f1_].edge0;	f11 = node[f1_].edge1;
        if ( isN(f1) ) { f10 = ~f10; f11 = ~f11; }
    }
    //// h0
    if ( f0lev > f1lev ) rec_sym_ijsub(f00, f1, lev);
    else if ( f0lev < f1lev ) rec_sym_ijsub(f0, f10, lev);
    else rec_sym_ijsub(f00, f10, lev);
    //// h1
    if ( sym_result.is_symm() ) {
        if ( f0lev > f1lev ) rec_sym_ijsub(f01, f1, lev);
        else if ( f0lev < f1lev ) rec_sym_ijsub(f0, f11, lev);
        else rec_sym_ijsub(f01, f11, lev);
    }

    /*-------- write to the result table --------*/
    RTWRITE( rtable+key, f0, f1 );
}

static void rec_svsym_ij(bddid f, int i, int j)
{
    //////// terminal cases ////////
    bddid f_ = rmN(f);
    if ( f_ == BDDF ) return;
    if ( node[f_].mark ) return;

    int flev = node[f_].lev;
    if ( flev < j ) return;

    //////// recursive computation ////////
    if ( flev > i ) {
        rec_svsym_ij(node[f_].edge0, i, j);
        if ( svsym_result.is_and() ) {
            rec_svsym_ij(node[f_].edge1, i, j);
        }
    }
    else if ( i == flev ) {
        bddid f0 = rstrtop(f_, BDDF);
        bddid f1 = rstrtop(f_, BDDT);
        rec_svsym_ijsub(f0, f1, j);
    }
    else {
        //// j <= flev < i, iが依存していないとき，jも依存していなければよい
        //// これはもうpath typeで調べてあるのでしなくても良い．
    }
    node[f_].mark = 1;
    //     printf("rec_sym_ij(%d, %d, %d) = %d, flev=%d\n", f, i, j, result, flev);
}

static void rec_svsym_ijsub(bddid f0, bddid f1, int lev)
{
    bddid sup = bddvarset(f0);	//// lev以下も調べるのでもったいない?
    bddid suprstr = zbddsubset(sup, lev); // subset0
    if ( sup != suprstr ) {//// bddrstr(f0,lev,0) != bddrstr(f0,lev,1)
        svsym_result.neg_skip0();
    }
    bddfree(sup); bddfree(suprstr);

    sup = bddvarset(f1);	//// lev以下も調べるのでもったいない?
    suprstr = zbddsubset(sup, lev); // subset0
    if ( sup != suprstr ) {//// bddrstr(f1,lev,0) != bddrstr(f1,lev,1)
        svsym_result.neg_skip1();
    }
    bddfree(sup); bddfree(suprstr);
}

static void rec_andExt(bddid f, VarSymm* symRel)
{
    bddid f_ = rmN(f);
    int flev = node[f_].lev;
    if ( flev <= 1 ) return;
    if ( node[f_].mark == 1 ) return;

    bddid f0 = rstrtop(f, BDDF);
    bddid f1 = rstrtop(f, BDDT);

    VarSymm* ptr = &(symRel[flev]);
    if ( ptr->is_symm() ) {	//// groupのtopのみにすべし
        if ( ptr->is_skip0() ) {
            bddid f01 = rstrtopup(f0, ~(flev-1));
            bddid f00 = rstrtopup(f0,   flev-1 );
            if ( f00 != f01 ) ptr->neg_skip0();
        }
        if ( ptr->is_skip1() ) {
            bddid f10 = rstrtopup(f1,   flev-1 );
            bddid f11 = rstrtopup(f1, ~(flev-1));
            if ( f10 != f11 ) ptr->neg_skip1();
        }
    }
    else symRel[flev].neg_sym();

    //// recursive
    rec_andExt(f0, symRel);
    rec_andExt(f1, symRel);
    node[f_].mark = 1;
}


//////////////////////////////////////////////////////////////////
//	xor factoring, and factoring, common literals
//////////////////////////////////////////////////////////////////

// 飛んでいるノードに対しても結果を書き込むので,
// compressしてからで無いと効率が悪い

enum { NG, OK };

static int* xorFactorResult;

static void recXorFactor(bddid f, int plevMinus1)
{
    //////// terminal cases ////////
    bddid f_ = rmN(f);
    int flev = node[f_].lev;
    for ( int i = plevMinus1; i>flev; i-- ) {
        xorFactorResult[i] = NG;
    }
    if ( flev == 0 ) return; // f_ == BDDF
    if ( node[f_].mark ) return;

    //////// recursive computation ////////
    bddid f0 = node[f_].edge0;
    bddid f1 = node[f_].edge1;
    if ( f0 != ~f1 ) xorFactorResult[flev] = NG;
    recXorFactor(f0, flev-1);
    recXorFactor(f1, flev-1);

    node[f_].mark = 1;
}

// f = x1 ^ x2 ^ h(x3, ..., xn) なら，resultは 1 2 となる．
Arrayint Bdd::xorFactor() const
{
    Arrayint result;
    if ( *this == Bdd::null ) return result;

    int n = top();
    xorFactorResult = new int[n+1];
    for ( int i = n; i>=1; i-- ) xorFactorResult[i] = OK;

    recXorFactor(edge, n-1);
    clearMark(edge);

    for ( int i = n; i>=1; i-- ) if ( xorFactorResult[i] == OK ) result.add(i);
    delete [] xorFactorResult;

    return result;
}

static int* andFactorResult0;
static int* andFactorResult1;

static void recAndFactor(bddid f, int plevMinus1)
{
    //////// terminal cases ////////
    if ( f == BDDF ) return;

    bddid f_ = rmN(f);
    int flev = node[f_].lev;
    for ( int i = plevMinus1; i>flev; i-- ) {
        andFactorResult0[i] = NG;
        andFactorResult1[i] = NG;
    }
    if ( flev == 0 ) return; // f == BDDT
    if ( (isN(f) && node[f_].mark & (1<<1)) || (!isN(f) && node[f_].mark & 1) ) return;

    //////// recursive computation ////////
    bddid f0 = node[f_].edge0;
    bddid f1 = node[f_].edge1;
    if ( isN(f) ) { f0 = ~f0; f1 = ~f1; }
    if ( f0 != BDDF ) {
        andFactorResult0[flev] = NG;
    }
    if ( f1 != BDDF ) {
        andFactorResult1[flev] = NG;
    }
    recAndFactor(f0, flev-1);
    recAndFactor(f1, flev-1);

    node[f_].mark |= isN(f) ? 1<<1 : 1;
}

// f = x1 & ~x2 & h(x3, ..., xn) なら，resultは 1 ~2 となる．
Arrayint Bdd::andFactor() const
{
    Arrayint result;
    if ( *this == Bdd::null ) return result;

    int i, n = top();
    andFactorResult0 = new int[n+1];
    andFactorResult1 = new int[n+1];
    for ( i = n; i>=1; i-- ) {
        andFactorResult0[i] = OK;
        andFactorResult1[i] = OK;
    }

    recAndFactor(edge, n-1);
    clearMark(edge);

    for ( i = n; i>=1; i-- ) {
        if ( andFactorResult0[i] == OK ) result.add(i);
        else if ( andFactorResult1[i] == OK ) result.add(~i);
    }
    delete [] andFactorResult0;
    delete [] andFactorResult1;

    return result;
}
