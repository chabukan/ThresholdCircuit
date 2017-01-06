#include <assert.h>
#include "bddnode.h"
#include "bddc.h"

// binary operation (binaryop.cc)
extern void	rtinit(int rtsize_power);
extern void	enlarge_rtable();
extern void	clear_rtable();
extern void	check_clear_rtable();
extern void	status_rtable();

// single operation (singleop.cc)
extern void	srtinit(int srtsize_power);
extern void	enlarge_srtable();
extern void	clear_srtable();
extern void	check_clear_srtable();
extern void	status_srtable();

extern int	bddshcompo(int f, int g, int var); // opbinary.C

//////////////////////////////////////////////////////////////////

static void enlarge_table() {
    enlarge_rtable();
    enlarge_srtable();
}

static void check_clear_table() {
    check_clear_rtable();
    check_clear_srtable();
}

// todo 未使用
// static void clear_table() {
//     clear_rtable();
//     clear_srtable();
// }

void bddalloc(int initsize_power, int maxsize_power)
{
    assert( initsize_power < 31 );
    assert( maxsize_power < 31 );
    int rtsize_power = initsize_power-1;
    int srtsize_power = initsize_power-1;

    alloc_bdd_node(initsize_power, maxsize_power);
    rtinit(rtsize_power);
    srtinit(srtsize_power);
    //     setEfunc(&enlarge_rtable, &clear_rtable);
    setEfunc(&enlarge_table, &check_clear_table);
}

void bddstatus() {
    status_rtable();
    status_srtable();
}

void bddgc() {
    gCollect();
}

int bddused() {
    return ncount;
}

bddid bddcopy(bddid f) {
    bddid f_ = rmN(f);
    if ( isNode(f_) ) node[f_].inc();
    return f;
}

bddid bddfree(bddid f) {
    bddid f_ = rmN(f);
    if ( isNode(f_) ) node[f_].dec();
    return f;
}

bddid bddvar(int lev) {
    if ( lev <= 0 || lev > MAXLEV )
        abort_bdd((char*)"in bddVar(): invalid argument");
    return getnode(BDDF, BDDT, lev);
}

int bddtop(int f)
{
    return node[rmN(f)].lev;	//// node[0].lev == 0
}

bddid bddrstrtop0(bddid f) {
    return bddcopy( ::rstrtop(f, BDDF) );
}
bddid bddrstrtop1(bddid f) {
    return bddcopy( ::rstrtop(f, BDDT) );
}

//// compute x_var * f1 + ~x_var * f0, where var is the level of x_var
bddid bddvarite(int var, int f1, int f0)
{
    int hh = bddshcompo(f0, f1, var);
    int h = bddlevshift(hh, var+1, -1);
    bddfree(hh);
    return h;
}

bddid bddcompose(bddid f, int lev, bddid g) { // f(x_lev = g)
    bddid f0 = bddrstr(f,  lev);
    bddid f1 = bddrstr(f, ~lev);
    bddid gf1 =   bddand( g, f1);
    bddid n_gf0 = bddand(~g, f0);
    bddid result = bddor( gf1, n_gf0 );
    bddfree(f0); bddfree(f1); bddfree(gf1); bddfree(n_gf0);
    return result;
}

//////////////////////////////////////////////////////////////////
//	bddsize
//////////////////////////////////////////////////////////////////

static int rec_size(int f)
{
    int f_ = rmN(f);
    if ( !isNode(f_) ) return 0;
    if ( node[f_].mark ) return 0;
    node[f_].mark = 1;
    return rec_size( node[f_].edge0 ) + rec_size( node[f_].edge1 ) + 1;
}

int bddsize(bddid f) {
    int sz = rec_size(f);
    clearMark(f);
    return sz;
}

static int rec_size(int f, int i, int j)
{
    int f_ = rmN(f);
    if ( !isNode(f_) ) return 0;
    if ( node[f_].mark ) return 0;

    node[f_].mark = 1;
    int flev = node[f_].lev;
    if ( i <= j ) {
        if ( flev < i ) return 0;
        else if ( j < flev )
            return rec_size( node[f_].edge0, i, j )
                + rec_size( node[f_].edge1, i, j );
        else	//// i <= flev <= j
            return rec_size( node[f_].edge0, i, j )
                + rec_size( node[f_].edge1, i, j ) + 1;
    }
    else {	//// i > j
        if ( j < flev && flev < i )
            return rec_size( node[f_].edge0, i, j )
            + rec_size( node[f_].edge1, i, j );
        else	//// flev <= j || i <= flev
            return rec_size( node[f_].edge0, i, j )
            + rec_size( node[f_].edge1, i, j ) + 1;
    }
}

//// size of from level i to level j
//// if ( i <= j ) size of i, i+1, ..., j-1, j
//// if ( i > j ) size of i, i+1, ..., n-1, n, 1, 2, ...., j-1, j
int bddsize(bddid f, int i, int j) {
    int sz = rec_size(f, i, j);
    clearMark(f);
    return sz;
}

int bddsize(bddid f, int lev) {
    int sz = rec_size(f, lev, lev);
    clearMark(f);
    return sz;
}

//////////////////////////////////////////////////////////////////
//	literal count
//////////////////////////////////////////////////////////////////

// literalsに加えられていく．必要ならば外側で初期化すること．
void zbddlitcount(bddid f, uint* literals)
{
    int f_ = rmN(f);
    if ( !isNode(f_) ) return;

    int lev = node[f_].lev;
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;

    literals[lev] += zbddcard(f1);

    zbddlitcount(f1, literals);
    zbddlitcount(f0, literals);
}

// literalsに加えられていく．必要ならば外側で初期化すること．
// max以下のみ数える
void zbddlitcount(bddid f, uint* literals, int max)
{
    int f_ = rmN(f);
    if ( !isNode(f_) ) return;

    int lev = node[f_].lev;
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;

    if ( lev <= max ) literals[lev] += zbddcard(f1);

    zbddlitcount(f1, literals, max);
    zbddlitcount(f0, literals, max);
}

// 0か1か2以上を区別するだけのzbddlitcount
void zbddmultilit(bddid f, char* literals)
{
    int f_ = rmN(f);
    if ( !isNode(f_) ) return;

    int lev = node[f_].lev;
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;

    if ( literals[lev] <=1 ) {
        literals[lev] += zbddmulti(f1);
    }

    zbddmultilit(f1, literals);
    zbddmultilit(f0, literals);
}

//////////////////////////////////////////////////////////////////
//	printbdl
//////////////////////////////////////////////////////////////////

static void rec_printbdl(int f, int invert, FILE* bdlout)
{
    int ff = invert ? ~f : f;

    if ( ff == BDDF ) fprintf(bdlout, " 0");
    else if ( ff == BDDT ) fprintf(bdlout, " 1");
    else {
        int f_ = rmN(f);
        if ( (isN(ff) && ( node[f_].mark & (1<<1) )) ||
             (!isN(ff) && ( node[f_].mark & 1 )) ) {
            if ( isN(ff) ) fprintf(bdlout, " $0%u", ~ff);
            else fprintf(bdlout, " $%u", ff);
            return;
        }

        node[f_].mark |= isN(ff) ? 1<<1 : 1;

        if ( isN(ff) ) fprintf(bdlout, "( $0%u =", ~ff);
        else fprintf(bdlout, "( $%u =", ff);
        int flev = node[f_].lev;
        fprintf(bdlout, " x%d", flev);
        rec_printbdl(node[f_].edge1, (isN(f) != invert), bdlout);
        fprintf(bdlout, " + ~x%d", flev);
        rec_printbdl(node[f_].edge0, (isN(f) != invert), bdlout);
        fprintf(bdlout, " )");
    }
}    

void bddprintbdl(bddid f, FILE* bdlout) {
    fprintf(bdlout, "A=");
    rec_printbdl(f, 0, bdlout);
    fprintf(bdlout, ";\n");
    clearMark(f);
}
