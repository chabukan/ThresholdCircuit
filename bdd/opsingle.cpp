#include <memory.h>
#include <assert.h>
#include <limits.h>
#include "bddnode.h"
#include "bddc.h"

/* bddの処理のうち，単項演算で演算結果テーブルを使うものを集めた． */

//////////////////////////////////////////////////////////////////

static int	bddrstrsft(int f, int lev); // BDDF: lev, BDDT: ~lev
static int      zbddsubsetsft(bddid f, int lev); // BDDF: lev, BDDT: ~lev
extern int	bddshcompo(int f, int g, int var); // opbinary.C
extern const int MAXLEVLOG; // bddnode.C

//////////////////////////////////////////////////////////////////

#define srthash(f, op)                          \
    ( ((op)^(f)^(f<<7)) & (srtsize-1) )

struct SResult { int	f, op, h; };
static SResult*		srtable;
static int		srtsize, srtref = 0, srthit = 0, srtwri = 0;

inline int SRTCMP(SResult* p, int f, int op) {
    srtref++;	return (p->op == op) && (p->f == f);
    //     srtref++;	return (p->f == f) && (p->op == op);
}
inline void SRTWRITE(SResult* p, int f, int op, int h) {
    srtwri++;	p->f = f;   p->op = op;   p->h = h;
}

enum { SNOP=0, COMMONLIT=1, RSTR=2, RSTRSFT=3, DENSE_R=4, SMOOTH=5, 
	   VARSET=6, LEVSHIFT=7, JUMPUD=8, LIT=9,
	   CHANGE=10, SUBSET=11, CARD=12,
	   SOP2FUNC=13, ZJUMPUP=14, SUBSETSFT=15, ZLEVSHIFT=16,
       MULTI=17, DELLITCUBE=18};
const int SOPLOG = 5;	//// because < (5th power of 2 = 32)
// urary op		RSTR, DENSE_R, SMOOTH, VARSET
// urary op (order)     RSTRSFT, LEVSHIFT, JUMPUD
// urary op (ZBDD)	LIT, CHANGE, SUBSET, CARD, SOP2FUNC, ZJUMPUP, SUBSETSFT, ZLEVSHIFT, COMMONLIT, MULTI, DELLITCUBE


void srtinit(int srtsize_power) {
    register int i;

    srtsize = 1<<srtsize_power;

    //     printf("sizeof(SResult) = %d\n", sizeof(SResult));
    srtable = new SResult[srtsize];
    if ( !srtable ) abort_bdd((char*)"in rtinit(): Out of Memory");
    for (i=srtsize-1; i>=0; i--) srtable[i].op = 0;
}

void enlarge_srtable() {
    struct SResult*	newsrtable;

    //////// enlarge srtable ////////
    newsrtable = new SResult[srtsize<<1];
    if ( !newsrtable ) abort_bdd((char*)"in enlarge_rtable(): Out of Memory\n");
    memcpy(newsrtable, srtable, sizeof(SResult)*srtsize);
    memcpy(newsrtable+srtsize, srtable, sizeof(SResult)*srtsize);
    delete [] srtable;
    srtable = newsrtable;
    srtsize<<=1;
}

void check_clear_srtable() {
    register int i;
    for ( i=srtsize; --i>=0; ) {
        SResult* srt_i = srtable+i;
        if ( srt_i->op ) {
            int f_ = rmN(srt_i->f);
            if ( f_ != BDDF && node[f_].lev == 0 ) {
                srt_i->op = SNOP;
                continue;
            }
            f_ = rmN(srt_i->h); 
            if ( f_ < 0 || nsize <= f_ ) continue; // LIT, CARDのため
            if ( f_ != BDDF && node[f_].lev == 0 ) srt_i->op = SNOP;
        }
    }
}

void clear_srtable() {
    register int i;
    for ( i=srtsize; --i>=0; ) srtable[i].op = SNOP;
    srtwri = 0;
}

void status_srtable() {
    register int i, srtuse;

    for ( i=srtsize-1, srtuse=0; i>=0; i-- )
        if ( srtable[i].op != 0 ) srtuse++;
    fprintf(stderr,
            "SRT hit/ref=%6d/%6d(%.4f) use/wri=%6d/%6d(%.4f) size=%6d\n",
            srthit, srtref, (float)srthit/srtref,
            srtuse, srtwri, (float)srtuse/srtwri, srtsize);
}

//////////////////////////////////////////////////////////////////

int bddsmooth(int f, int lev)
{
    //////// terminal cases ////////
    int f_ = rmN(f);
    if (f_ == BDDF) return f;
    int flev = node[f_].lev;
    if ( flev < lev ) return (node[f_].inc(), f);

    //////// read the result table ////////
    int op_lev = (lev<<SOPLOG) ^ SMOOTH;
    int key = srthash( f, op_lev );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f, op_lev) ) {
        srthit++;
        return bddcopy(srt_key->h);
    }

    //////// recursive computation ////////
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    if ( isN(f) ) { f0 = ~f0; f1 = ~f1; }
    int h;
    if ( flev == lev ) h = ~bddand( ~f0, ~f1 ); //// same lev: f0 OR f1
    else h = getnode(bddsmooth(f0, lev), bddsmooth(f1, lev), flev);
    //////// write to the result table ////////
    SRTWRITE(srtable+key, f, op_lev, h);
    return h;
}

int bddrstr(int f, int lev) // lev: BDDF, ~lev: BDDT を代入
{
    //////// terminal cases ////////
    int f_ = rmN(f);
    if (f_ == BDDF) return f;
    int flev = node[f_].lev;
    int lev_ = rmN(lev);
    if ( flev < lev_ ) return (node[f_].inc(), f);
    else if ( flev == lev_ ) return bddcopy( rstrtop(f, lev) );
    // 本当は，BDDFかBDDTを代入するが，levでも同じ働きをするはず．

    //////// read the result table ////////
    int h_;
    int op_lev = (lev<<SOPLOG) ^ RSTR;
    int key = srthash( f_, op_lev );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, op_lev) ) {
        srthit++;
        h_ = srt_key->h;
        return bddcopy( isN(f) ? ~h_ : h_ );
    }

    //////// recursive computation ////////
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    h_ = getnode(bddrstr(f0, lev), bddrstr(f1, lev), flev);
    //////// write to the result table ////////
    SRTWRITE(srtable+key, f_, op_lev, h_);
    return isN(f) ? ~h_ : h_;
}

const u_int FULLDENSE = 1<<31;
//// I do not know the value of FULLDENSE is OK.
//// (1<<30) may be better.
////
//// 1996.1.9
//// bdddense(f) -> bdddense_rstr(f, 0)
////
u_int bdddense_rstr(int f, int lev) // lev: BDDF, ~lev: BDDT
{
    //////// terminal cases ////////
    if (f == BDDF) return 0;
    else if (f == BDDT) return FULLDENSE;

    int f_ = rmN(f);
    int flev = node[f_].lev;
    int lev_ = rmN(lev);
    if ( flev < lev_ ) { lev_ = 0; }

    //////// read the result table ////////
    int op_lev = (lev<<SOPLOG) ^ DENSE_R;
    int key = srthash( f_, op_lev );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, op_lev) ) {
        srthit++;
        u_int h = (u_int) srt_key->h;
        //    printf("chhit: f=%4d h=%10x op=%d\n", f_, h, DENSE_R);
        return isN(f) ? FULLDENSE-h : h;
    }

    //////// recursive computation ////////
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    u_int h;
    if ( flev == lev_ )
        h = isN(lev) ? bdddense_rstr(f1, 0) : bdddense_rstr(f0, 0);
    else {
        u_int f0dense = bdddense_rstr(f0, lev);
        u_int f1dense = bdddense_rstr(f1, lev);
        u_int lsb = (f0dense & 1) & (f1dense & 1);
        h = (f0dense>>1) + (f1dense>>1) + lsb;
    }
    //////// write to the result table ////////
    SRTWRITE(srtable+key, f_, op_lev, (int)h);
    //     printf("write: f=%4d h=%10x op=%d\n", f_, h, DENSE_R);
    return isN(f) ? FULLDENSE-h : h;
}

u_int bdddense(bddid f)
{
    return bdddense_rstr(f, 0);
}

//// shift variable nodes upper or equal to "lev"
//// "degree": shift amount. +: up, -: down.
////
int bddlevshift(int f, int lev, int degree)
{
    //////// terminal cases ////////
    int f_ = rmN(f);
    if ( f_ == BDDF ) return f;
    int flev = node[f_].lev;
    if ( flev < lev ) return (node[f_].inc(), f);

    //////// read the result table ////////
    int h;
    int op_levdeg = ( ((degree<<MAXLEVLOG) ^ lev)<<SOPLOG ) ^ LEVSHIFT;
    int key = srthash( f_, op_levdeg );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, op_levdeg) ) {
        srthit++;
        h = srt_key->h;
        node[h].inc();	//// h must be positive
        if ( isN(f) ) h = ~h;
        return h;
    }
    //////// recursive computation ////////
    int h0 = bddlevshift(node[f_].edge0, lev, degree);
    int h1 = bddlevshift(node[f_].edge1, lev, degree);

    assert( flev+degree > node[rmN(h0)].lev );	//// assert for down
    assert( flev+degree > node[rmN(h1)].lev );	//// assert for down

    h = getnode(h0, h1, flev+degree);
    //////// write to the result table ////////
    SRTWRITE(srtable+key, f_, op_levdeg, h);
    return isN(f) ? ~h : h;
}

//// move nodes of level i to level j
//// i<j: jumpup, decrease levels of nodes of level from i+1 to j by 1
////		use static bddrstrsft
//// i>j: jumpdown, increase levels of nodes of level from j to i+1 by 1
////		use static bddshcompo
////
int bddjumpud(int f, int i, int j)
{
    //////// terminal cases ////////
    int f_ = rmN(f);
    if ( f_ == BDDF ) return f;
    int flev = node[f_].lev;

    if ( (i == j) || ((flev<i)&&(flev<j)) ) {
        return (node[f_].inc(), f);
    }
    else if ( (j<i) && (flev<i) ) { // j < flev < i
        return bddlevshift(f, j, 1);
    }
    else if ( (j<i) && (flev==i) ) { // j < i == flev
        int h = bddshcompo(node[f_].edge0, node[f_].edge1, j);
        return isN(f) ? ~h : h;  
    }

    //////// read the result table ////////
    int h;
    int op_ij = ( ((i<<MAXLEVLOG) ^ j)<<SOPLOG ) ^ JUMPUD;
    int key = srthash( f_, op_ij );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, op_ij) ) {
        srthit++;
        h = srt_key->h;
        node[h].inc();	//// h must be positive
        if ( isN(f) ) h = ~h;
        return h;
    }

    //////// recursive computation ////////
    if ( (flev>i) && (flev>j) ) {
        int h0 = bddjumpud(node[f_].edge0, i, j);
        int h1 = bddjumpud(node[f_].edge1, i, j);
        h = getnode(h0, h1, flev);
    }
    else { // i < flev <= j
        int h0 = bddrstrsft(f_, i);
        int h1 = bddrstrsft(f_, ~i);
        h = getnode(h0, h1, j);
    }
    //////// write to the result table ////////
    SRTWRITE(srtable+key, f_, op_ij, h);
    return isN(f) ? ~h : h;  
}

//// compute f[x_lev = val] and nodes of level > lev are down-shifted 1 level.
//// val: BDDF: lev, val: BDDT: ~lev
static int bddrstrsft(int f, int lev)
{
    //////// terminal cases ////////
    int f_ = rmN(f);
    if (f_ == BDDF) return f;
    int flev = node[f_].lev;
    int lev_ = rmN(lev);
    if ( flev < lev_ ) return (node[f_].inc(), f);
    else if ( flev == lev_ ) return bddcopy( rstrtop(f, lev) );
    // 本当は，BDDFかBDDTを代入するが，levでも同じ働きをするはず．

    //////// read the result table ////////
    int h;
    int op_lev = (lev<<SOPLOG) ^ RSTRSFT;
    int key = srthash( f_, op_lev );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, op_lev) ) {
        srthit++;
        h = srt_key->h;
        if ( isN(f) ) h = ~h;
        return bddcopy(h);
    }
    //////// recursive computation //////// flev > lev
    int h0 = bddrstrsft(node[f_].edge0, lev);
    int h1 = bddrstrsft(node[f_].edge1, lev);
    h = getnode(h0, h1, flev-1);

    //////// write to the result table ////////
    SRTWRITE(srtable+key, f_, op_lev, h);
    return isN(f) ? ~h : h;
}

//////////////////////////////////////////////////////////////////

int bddvarset(int f)
{
    assert( f != BDDNULL );
    //////// terminal cases ////////
    if ( f == BDDF || f == BDDT ) return ZBDDE;

    //////// read the result table ////////
    int f_ = rmN(f);
    int key = srthash( f_, VARSET );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, VARSET) ) {
        srthit++;
        int h = srt_key->h;	//// h must be positive and var node
        return (node[h].inc(), h);
    }

    //////// recursive computation ////////
    int flev = node[f_].lev;
    int f0sup = bddvarset(node[f_].edge0);
    int f1sup = bddvarset(node[f_].edge1);
    int h = getzbdd(zbddunion(f0sup, f1sup), ZBDDB, flev);
    bddfree(f0sup);
    bddfree(f1sup);
    //////// write to the result table ////////
    SRTWRITE(srtable+key, f_, VARSET, h);
    return h;
}

int zbddchange(int f, int lev)
{
    //////// terminal cases ////////
    int f_ = rmN(f);
    int flev = node[f_].lev;
    if ( flev < lev ) return getzbdd(ZBDDE, bddcopy(f), lev);

    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    if ( isN(f) ) { f0 = ~f0; }
    if ( flev == lev ) return getzbdd(bddcopy(f1), bddcopy(f0), lev);

    //////// read the result table ////////
    int h;
    int op_lev = (lev<<SOPLOG) ^ CHANGE;
    int key = srthash( f, op_lev );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f, op_lev) ) {
        srthit++;
        h = srt_key->h;
        return bddcopy( h );
    }

    //////// recursive computation ////////
    h =  getzbdd( zbddchange(f0,lev), zbddchange(f1,lev), flev);

    //////// write to the result table ////////
    SRTWRITE(srtable+key, f, op_lev, h);
    return h;
}

int zbddsubset(int f, int lev) // subset0: lev, subset1: ~lev
{
    //////// terminal cases ////////
    if ( f == ZBDDE ) return ZBDDE;
    else if ( f == ZBDDB ) return isN(lev) ? ZBDDE : ZBDDB;

    int f_ = rmN(f);
    int flev = node[f_].lev;
    int lev_ = rmN(lev);
    if ( flev < lev_ ) {
        if ( isN(lev) ) return ZBDDE;
        else return (node[f_].inc(), f);
    }
    else if ( flev == lev_ )
        return bddcopy( subsettop(f, lev) );
    // 本当は，BDDFかBDDTを代入するが，levでも同じ働きをするはず．

    //////// read the result table ////////
    int h_;
    int op_lev = (lev<<SOPLOG) ^ SUBSET;
    int key = srthash( f_, op_lev );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, op_lev) ) {
        srthit++;
        h_ = srt_key->h;
        return bddcopy( (isN(f) && !isN(lev)) ? ~h_ : h_ );
    }

    //////// recursive computation ////////
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    h_ = getzbdd(zbddsubset(f0, lev), zbddsubset(f1, lev), flev);
    //////// write to the result table ////////
    SRTWRITE(srtable+key, f_, op_lev, h_);
    return (isN(f) && !isN(lev)) ? ~h_ : h_;
}

//// compute f[x_lev = val] and nodes of level > lev are down-shifted 1 level.
//// val: BDDF: lev, val: BDDT: ~lev  (ZBDD)
int zbddsubsetsft(int f, int lev)
{
    //////// terminal cases ////////
    if ( f == ZBDDE ) return ZBDDE;
    else if ( f == ZBDDB ) return isN(lev) ? ZBDDE : ZBDDB;

    int f_ = rmN(f);
    int flev = node[f_].lev;
    int lev_ = rmN(lev);
    if ( flev < lev_ ) {
        if ( isN(lev) ) return ZBDDE;
        return (node[f_].inc(), f);
    }
    else if ( flev == lev_ ) return bddcopy( subsettop(f, lev) );
    // 本当は，BDDFかBDDTを代入するが，levでも同じ働きをするはず．

    //////// read the result table ////////
    int h_;
    int op_lev = (lev<<SOPLOG) ^ SUBSETSFT;
    int key = srthash( f_, op_lev );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, op_lev) ) {
        srthit++;
        h_ = srt_key->h;
        return bddcopy( (isN(f) && !isN(lev)) ? ~h_ : h_ );
    }
    //////// recursive computation //////// flev > lev
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    h_ = getzbdd(zbddsubsetsft(f0, lev), zbddsubsetsft(f1, lev), flev-1);
    //////// write to the result table ////////
    SRTWRITE(srtable+key, f_, op_lev, h_);
    return (isN(f) && !isN(lev)) ? ~h_ : h_;
}

uint zbddcard(int f)
{
    /*-------- terminal cases --------*/
    if ( f == ZBDDE ) return 0;
    else if ( f == ZBDDB ) return 1;

    /*-------- read the result table --------*/
    int f_ = rmN(f);
    int key = srthash(f_, CARD);
    SResult* srt_key = srtable+key;
    uint h_;
    if ( SRTCMP(srt_key, f_, CARD) ) {
        srthit++;
        h_ = srt_key->h;
        return ( isN(f) ) ? h_+1 : h_;
    }
    /*-------- recursive computation --------*/
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    h_ = zbddcard(f0) + zbddcard(f1);
    /*-------- write to the result table --------*/
    SRTWRITE( srtable+key, f_, CARD, h_ );
    return isN(f) ? h_+1 : h_;
}

uint zbddcard(bddid f, int lev) // levを含む集合の要素数
{
    /*-------- terminal cases --------*/
    int f_ = rmN(f);
    int flev = node[f_].lev;
    if ( flev < lev ) return 0;

    /*-------- read the result table --------*/
    int op_lev = (lev<<SOPLOG) ^ CARD;
    int key = srthash(f_, op_lev);
    SResult* srt_key = srtable+key;
    uint h_;
    if ( SRTCMP(srt_key, f_, op_lev) ) {
        srthit++;
        h_ = srt_key->h;
        return ( isN(f) ) ? h_+1 : h_;
    }

    /*-------- recursive computation --------*/
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    if ( flev == lev ) h_ = zbddcard(f1);
    else h_ = zbddcard(f0,lev) + zbddcard(f1,lev);

    /*-------- write to the result table --------*/
    SRTWRITE( srtable+key, f_, op_lev, h_ );
    return isN(f) ? h_+1 : h_;
}

int zbddmulti(int f) // zbddcard(f)の結果が0か1か2以上か
{
    /*-------- terminal cases --------*/
    if ( f == ZBDDE ) return 0;
    else if ( f == ZBDDB ) return 1;

    /*-------- read the result table --------*/
    int f_ = rmN(f);
    int key = srthash(f_, CARD);
    SResult* srt_key = srtable+key;
    int h, h_;
    if ( SRTCMP(srt_key, f_, CARD) ) {
        srthit++;
        h_ = srt_key->h;
        h = ( isN(f) ) ? h_+1 : h_;
        if ( h <= 1 ) return h;
        else return 2;
    }
    key = srthash(f_, MULTI);
    srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, MULTI) ) {
        srthit++;
        h_ = srt_key->h;
        h = ( isN(f) ) ? h_+1 : h_;
        if ( h <= 1 ) return h;
        else return 2;
    }
    /*-------- recursive computation --------*/
    int f1 = node[f_].edge1;
    h_ = zbddmulti(f1);
    if ( h_ <= 1 ) {
        int f0 = node[f_].edge0;
        h_ += zbddmulti(f0);
    }
    if ( h_ >= 2 ) h_ = 2;
    /*-------- write to the result table --------*/
    SRTWRITE( srtable+key, f_, MULTI, h_ );
    h =  isN(f) ? h_+1 : h_;
    return h;
}

int zbddmulti(bddid f, int lev) // levを含む集合に関する
{
    /*-------- terminal cases --------*/
    int f_ = rmN(f);
    int flev = node[f_].lev;
    if ( flev < lev ) return 0;

    /*-------- read the result table --------*/
    int op_lev = (lev<<SOPLOG) ^ CARD;
    int key = srthash(f_, op_lev);
    SResult* srt_key = srtable+key;
    int h, h_;
    if ( SRTCMP(srt_key, f_, op_lev) ) {
        srthit++;
        h_ = srt_key->h;
        h = ( isN(f) ) ? h_+1 : h_;
        if ( h <= 1 ) return h;
        else return 2;
    }
    op_lev = (lev<<SOPLOG) ^ MULTI;
    key = srthash(f_, op_lev);
    srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, op_lev) ) {
        srthit++;
        h_ = srt_key->h;
        h = ( isN(f) ) ? h_+1 : h_;
        if ( h <= 1 ) return h;
        else return 2;
    }

    /*-------- recursive computation --------*/
    int f1 = node[f_].edge1;
    if ( flev == lev ) h_ = zbddmulti(f1);
    else h_ = zbddmulti(f1,lev);
    if ( h_ <= 1 ) {
        int f0 = node[f_].edge0;
        if ( flev == lev ) ;
        else h_ += zbddmulti(f0,lev);
    }
    if ( h_ >= 2 ) h_ = 2;
    /*-------- write to the result table --------*/
    SRTWRITE( srtable+key, f_, op_lev, h_ );
    h =  isN(f) ? h_+1 : h_;
    return h;
}

int zbddlit(int f)
{
    /*-------- terminal cases --------*/
    int f_ = rmN(f);
    int flev = node[f_].lev;
    if ( flev <= 0 ) return 0;

    /*-------- read the result table --------*/
    int key = srthash(f_, LIT);
    SResult* srt_key = srtable+key;
    int h_;
    if ( SRTCMP(srt_key, f_, LIT) ) {
        srthit++;
        h_ = srt_key->h;
        return h_;
    }
    /*-------- recursive computation --------*/
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;

    // INT_MAX>>2 より大きければ，INT_MAXを返す．
    int f0lit = zbddlit(f0);
    if ( f0lit > (INT_MAX>>2) ) return INT_MAX;
    int f1lit = zbddlit(f1);
    if ( f1lit > (INT_MAX>>2) ) return INT_MAX;
    int f1card = zbddcard(f1);
    if ( f1card > (INT_MAX>>2) ) return INT_MAX;
    h_ = f0lit + f1lit + f1card;

    /*-------- write to the result table --------*/
    SRTWRITE( srtable+key, f_, LIT, h_ );
    return h_;
}

/* Sop表現が f である関数を返す．Sop表現は sop.h に従う．
 すなわち，変数varに対し，posLit: 2*var，negLit: 2*var-1．*/
int zbddsop2func(int f)
{
    /*-------- terminal cases --------*/
    if ( f == ZBDDE ) return BDDF;
    else if ( (f == ZBDDB) || isN(f) ) return BDDT;
    // 否定エッジがあるということは，Largest Cube があるということ → BDDT

    /*-------- read the result table --------*/
    int f_ = rmN(f);
    int key = srthash(f_, SOP2FUNC);
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, SOP2FUNC) ) {
        srthit++;
        return bddcopy(srt_key->h);
    }
    /*-------- recursive computation --------*/
    int flev = node[f_].lev;
    int fvar = (flev+1) >> 1; // Sop表現での変数番号
    int	negLit = (flev & 1);  // 偶数: posLit，奇数: negLit

    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;

    /* flevがposLitのとき，F = F0 + F1*fvar
     flevがnegLitのとき，F = F0 + F1*fvar' */
    int F1x;
    if ( !negLit ) {
        assert( node[rmN(f1)].lev < (fvar<<1)-1 );
        F1x = getnode( BDDF, zbddsop2func(f1), fvar );
    }
    else {
        assert( node[rmN(f1)].lev < (fvar<<1)-1 );
        F1x = getnode( zbddsop2func(f1), BDDF, fvar );
    }
    int F0 = zbddsop2func(f0);
    int h_ = ~bddand( ~F0, ~F1x ); // bddor
    bddfree(F0);
    bddfree(F1x);

    /*-------- write to the result table --------*/
    SRTWRITE( srtable+key, f_, SOP2FUNC, h_ );
    return h_;
}

/* ZBDDは縮約規則がBDDと違うため，ZBDD専用のlevshift */
//// shift variable nodes upper or equal to "lev"
//// "degree": shift amount. +: up, -: down.
////
int zbddlevshift(int f, int lev, int degree)
{
    //////// terminal cases ////////
    int f_ = rmN(f);
    if ( f_ == BDDF ) return f;
    int flev = node[f_].lev;
    if ( flev < lev ) return (node[f_].inc(), f);

    //////// read the result table ////////
    int h;
    int op_levdeg = ( ((degree<<MAXLEVLOG) ^ lev)<<SOPLOG ) ^ ZLEVSHIFT;
    int key = srthash( f_, op_levdeg );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, op_levdeg) ) {
        srthit++;
        h = srt_key->h;
        node[h].inc();	//// h must be positive
        if ( isN(f) ) h = ~h;
        return h;
    }
    //////// recursive computation ////////
    int h0 = zbddlevshift(node[f_].edge0, lev, degree);
    int h1 = zbddlevshift(node[f_].edge1, lev, degree);

    assert( flev+degree > node[rmN(h0)].lev );	//// assert for down
    assert( flev+degree > node[rmN(h1)].lev );	//// assert for down

    h = getzbdd(h0, h1, flev+degree);
    //////// write to the result table ////////
    SRTWRITE(srtable+key, f_, op_levdeg, h);
    return isN(f) ? ~h : h;
}

/* ZBDDは縮約規則がBDDと違うため，これ専用のjumpupルーチンを作る必要があった */
//// move nodes of level i to level j (ZBDD)
//// i<j: jumpup, decrease levels of nodes of level from i+1 to j by 1
////		use static zbddsubsetsft
int zbddjumpup(int f, int i, int j)
{
    //////// terminal cases ////////
    int f_ = rmN(f);
    if ( f_ == BDDF ) return f;
    int flev = node[f_].lev;

    if ( (i == j) || ((flev<i)&&(flev<j)) ) {
        return (node[f_].inc(), f);
    }

    //////// read the result table ////////
    int h;
    int op_ij = ( ((i<<MAXLEVLOG) ^ j)<<SOPLOG ) ^ ZJUMPUP;
    int key = srthash( f_, op_ij );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, op_ij) ) {
        srthit++;
        h = srt_key->h;
        node[h].inc();	//// h must be positive
        if ( isN(f) ) h = ~h;
        return h;
    }

    //////// recursive computation ////////
    if ( (flev>i) && (flev>j) ) {
        int h0 = zbddjumpup(node[f_].edge0, i, j);
        int h1 = zbddjumpup(node[f_].edge1, i, j);
        h = getzbdd(h0, h1, flev);
    }
    else { // i < flev <= j
        int h0 = zbddsubsetsft(f_, i);
        int h1 = zbddsubsetsft(f_, ~i);
        h = getzbdd(h0, h1, j);
    }
    //////// write to the result table ////////
    SRTWRITE(srtable+key, f_, op_ij, h);
    return isN(f) ? ~h : h;  
}

// fのすべてのcubeに出てくるlitの集合を返す
bddid zbddcommonlit(bddid f)
{
    //////// terminal cases ////////
    if ( isN(f) ) return ZBDDE;
    // 以下では f は必ず正
    int flev = node[f].lev;
    if ( flev == 0 ) return ZBDDE;

    //////// read the result table ////////
    int h;
    int key = srthash( f, COMMONLIT );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f, COMMONLIT) ) {
        srthit++;
        h = srt_key->h;
        node[h].inc();	//// h must be positive
        return h;
    }

    //////// recursive computation ////////
    int f0 = node[f].edge0;
    int f1 = node[f].edge1;
    int h1 = zbddcommonlit(f1);
    if ( f0 == ZBDDE ) { // flevを付け加える
        int self = zbddchange(ZBDDB, flev);
        h = zbddunion(self, h1);
        bddfree(self);
    }
    else { // f0とf1の積集合
        int h0 = zbddcommonlit(f0);
        h = zbddintersec(h0, h1);
        bddfree(h0);
    }
    bddfree(h1);

    //////// write to the result table ////////
    SRTWRITE(srtable+key, f, COMMONLIT, h);
    return h;
}

// 和形による単一リテラルの集合を積形に変換
// a + b + c -> abc
bddid zbddsum2product(bddid f)
{
    //////// terminal cases ////////
    if ( f == ZBDDE ) return ZBDDB; // 空集合はbaseに
    assert( !isN(f) ); // 以下では f は必ず正
    int f0 = node[f].edge0;
    assert( node[f].edge1 == ZBDDB ); // 単一リテラルの和形集合なので

    if ( f0 == ZBDDE ) {
        return (node[f].inc(), f);
    }
    else {
        int flev = node[f].lev;
        bddid hsub = zbddsum2product(f0);
        return getzbdd(ZBDDE, hsub, flev);
    }
}

bddid zbddDelLitCube(bddid f)
{
    int f_ = rmN(f);
    int flev = node[f_].lev;
    //////// terminal cases ////////
    if ( flev == 0 ) return f;

    //////// read the result table ////////
    int h_;
    int key = srthash( f_, DELLITCUBE );
    SResult* srt_key = srtable+key;
    if ( SRTCMP(srt_key, f_, DELLITCUBE) ) {
        srthit++;
        h_ = srt_key->h;
        node[h_].inc();
        return ( isN(f) ) ? ~h_ : h_;
    }

    //////// recursive computation ////////
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    int h0 = zbddDelLitCube(f0);
    if ( f1 == ZBDDB ) {
        h_ = h0;
    }
    else {
        h_ = getzbdd(h0, bddcopy(f1), flev);
    }

    //////// write to the result table ////////
    SRTWRITE(srtable+key, f_, DELLITCUBE, h_);
    return ( isN(f) ) ? ~h_ : h_;
}
