#include <memory.h>
#include <assert.h>
#include "bddnode.h"
#include "bddc.h"

/* bddの処理のうち，2項演算で演算結果テーブルを使うものを集めた． */

//////////////////////////////////////////////////////////////////

#define rthash(f, g, op)                            \
    ( ( ((op)<<7)^(f)^((f)<<4)^(g) ) & (rtsize-1) )

struct Result {	 int	f, g, op, h; };
static Result*		rtable;
static int		rtsize, rtref = 0, rthit = 0, rtwri = 0;

inline int RTCMP(Result* p, int f, int g, int op) {
    rtref++;	return  (p->op == op) && (p->f == f) && (p->g == g);
    //     rtref++;	return (p->f == f) && (p->g == g) && (p->op == op);
}
inline void RTWRITE(Result* p, int f, int g, int op, int h) {
    rtwri++; 	p->f = f;   p->g = g;   p->op = op;   p->h = h;
}

enum { NOP=0, B_AND=1, COMPRESS=2, B_XOR=3, SHCOMPO=4, INTER=5, COFACT=6, 
	   UNION=7, INTERSEC=8, DIFF=9, PRODUCT=10, DIVIDE=11 };
const int OPLOG = 5;	//// 2^5 -1 = 31 まで
// binary op (commutative)	B_AND, B_XOR, INTER
// binary op 			COFACT, SHCOMPO, COMPRESS
// binary op (ZBDD)		UNION, INTERSEC, DIFF, PRODUCT, DIVIDE


void rtinit(int rtsize_power) {
    register int i;

    rtsize = 1<<rtsize_power;

    //     printf("sizeof(Result) = %d\n", sizeof(Result));
    rtable = new Result[rtsize];
    if ( !rtable ) abort_bdd((char*)"in rtinit(): Out of Memory");
    for (i=rtsize-1; i>=0; i--)	rtable[i].op = 0;
}

void enlarge_rtable() {
    struct Result*	newrtable;

    //////// enlarge rtable ////////
    newrtable = new Result[rtsize<<1];
    if ( !newrtable ) abort_bdd((char*)"in enlarge_rtable(): Out of Memory\n");
    memcpy(newrtable, rtable, sizeof(Result)*rtsize);
    memcpy(newrtable+rtsize, rtable, sizeof(Result)*rtsize);
    delete [] rtable;
    rtable = newrtable;
    rtsize<<=1;
}

void check_clear_rtable() {
    register int i;
    for ( i=rtsize; --i>=0; ) {
        Result* rt_i = rtable+i;
        if ( rt_i->op ) {
            int f_ = rmN(rt_i->h);
            if ( f_ != BDDF && node[f_].lev == 0 ) {
                rt_i->op = NOP;
                continue;
            }
            f_ = rmN(rt_i->f);
            if ( f_ != BDDF && node[f_].lev == 0 ) {
                rt_i->op = NOP;
                continue;
            }
            f_ = rmN(rt_i->g);
            if ( f_ != BDDF && node[f_].lev == 0 ) rt_i->op = NOP;
        }
    }
}

void clear_rtable() {
    register int i;
    for ( i=rtsize; --i>=0; ) rtable[i].op = NOP;
    rtwri = 0;
}

void status_rtable() {
    register int i, rtuse;

    for ( i=rtsize-1, rtuse=0; i>=0; i-- )
        if ( rtable[i].op != 0 ) rtuse++;
    fprintf(stderr,
            "RT hit/ref=%6d/%6d(%.4f) use/wri=%6d/%6d(%.4f) size=%6d\n",
            rthit, rtref, (float)rthit/rtref,
            rtuse, rtwri, (float)rtuse/rtwri, rtsize);
}

//////////////////////////////////////////////////////////////////

bddid bddreadcache(int op, bddid f, bddid g)
{
    int key = rthash(f, g, op);
    Result* rt_key = rtable+key;
    if ( RTCMP( rt_key, f, g, op ) ) {
        rthit++;
        return bddcopy( rt_key->h );
    }
    else {
        return BDDNULL;
    }
}

void bddwritecache(int op, bddid f, bddid g, bddid h)
{
    int key = rthash(f, g, op);
    RTWRITE( rtable+key, f, g, op, h );
}

//////////////////////////////////////////////////////////////////

int bddand(int f, int g)
{
    /*-------- terminal cases --------*/
    if ( f == ~g || f == BDDF || g == BDDF) return BDDF;
    else if ( f == g || g == BDDT ) return bddcopy(f);
    else if ( f == BDDT ) return bddcopy(g);

    /*-------- read the result table --------*/
    int f_ = rmN(f);
    int g_ = rmN(g);
    int flev = node[f_].lev;
    int glev = node[g_].lev;
    int h, swap=0;
    if ( (flev < glev) || ((flev == glev) && (f_ < g_)) )
	{ h = f; f = g; g = h; swap = 1; }

    int key = rthash(f, g, B_AND);
    Result* rt_key = rtable+key;
    if ( RTCMP( rt_key, f, g, B_AND ) ) {
        rthit++;
        return bddcopy( rt_key->h );
    }
    /*-------- recursive computation --------*/
    if ( swap ) {
        h = f_; f_ = g_; g_ = h;
        int tmp = flev; flev = glev; glev = tmp;
    }
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    if ( isN(f) ) { f0 = ~f0; f1 = ~f1; }
    if ( flev > glev ) 
        h = getnode(bddand(f0,g), bddand(f1,g), flev);
    else { // flev == glev
        int g0 = node[g_].edge0;
        int g1 = node[g_].edge1;
        if ( isN(g) ) { g0 = ~g0; g1 = ~g1; }
        h = getnode(bddand(f0,g0), bddand(f1,g1), flev);
    }
    /*-------- write to the result table --------*/
    RTWRITE( rtable+key, f, g, B_AND, h );
    return h;
}

int bddxor(int f, int g)
{
    //////// terminal cases ////////
    if (f == g) return BDDF;
    else if (f == ~g ) return BDDT;
    else if (f == BDDF) return bddcopy(g);
    else if (g == BDDF) return bddcopy(f);
    else if (f == BDDT) return bddcopy(~g);
    else if (g == BDDT) return bddcopy(~f);

    //////// read the result table ////////
    int f_ = rmN(f);
    int g_ = rmN(g);
    int flev = node[f_].lev;
    int glev = node[g_].lev;
    int h, swap=0;
    if ( (flev < glev) || ((flev == glev) && (f_ < g_)) ) {
        h = f; f = g; g = h;
        h = f_; f_ = g_; g_ = h;
        swap = 1;
    }

    int key = rthash( f_, g_, B_XOR );
    Result* rt_key = rtable+key;
    if ( RTCMP( rt_key, f_, g_, B_XOR ) ) {
        rthit++;
        h = rt_key->h;
        return ( isN(f) ^ isN(g) ) ? bddcopy(~h) : bddcopy(h);
    }

    //////// recursive computation ////////
    if ( swap ) { int tmp = flev; flev = glev; glev = tmp; }

    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    if ( isN(f) ) { f0 = ~f0; f1 = ~f1; }
    if ( flev > glev ) 
        h = getnode(bddxor(f0,g), bddxor(f1,g), flev);
    else { // flev == glev
        int g0 = node[g_].edge0;
        int g1 = node[g_].edge1;
        if ( isN(g) ) { g0 = ~g0; g1 = ~g1; }
        h = getnode(bddxor(f0,g0), bddxor(f1,g1), flev);
    }
    //////// write to the result table ////////
    RTWRITE( rtable+key, f_, g_, B_XOR, ( isN(f) ^ isN(g) ) ? ~h : h );
    return h;
}

int bddinter(int f, int g)
{
    //////// terminal cases ////////
    if ( (f == ~g) || (f == BDDF) || (g == BDDF) ) return 0;
    else if ( (f == g) || (f == BDDT) || (g == BDDT) ) return 1;

    //////// read the result table ////////
    int f_ = rmN(f);
    int g_ = rmN(g);
    int flev = node[f_].lev;
    int glev = node[g_].lev;
    int h, swap=0;
    if ( (flev < glev) || ((flev == glev) && (f_ < g_)) )       
    { h = f; f = g; g = h; swap = 1; }

    int key = rthash(f, g, INTER);
    Result* rt_key = rtable+key;
    if ( RTCMP(rt_key, f, g, INTER) ) {
        rthit++;
        return rt_key->h;
    }
    rt_key = rtable + rthash(f, g, B_AND);
    if ( RTCMP(rt_key, f, g, B_AND) ) {
        rthit++;
        return ( rt_key->h == BDDF ) ? 0 : 1;
    }

    //////// recursive computation ////////
    if ( swap ) {
        h = f_; f_ = g_; g_ = h;
        int tmp = flev; flev = glev; glev = tmp;
    }

    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    if ( isN(f) ) { f0 = ~f0; f1 = ~f1; }
    if ( flev > glev ) {
        h = bddinter(f0, g) || bddinter(f1, g);
    }
    else { // flev == glev
        int g0 = node[g_].edge0;
        int g1 = node[g_].edge1;
        if ( isN(g) ) { g0 = ~g0; g1 = ~g1; }
        h = bddinter(f0, g0) || bddinter(f1, g1);
    }
    //////// write to the result table ////////
    RTWRITE(rtable+key, f, g, INTER, h);	//// key: rthash of INTER
    return h;
}

int bddcofact(int f, int g)
{
    //////// terminal cases ////////
    if ( (f == ~g) || (g == BDDF) || (f == BDDF) ) return BDDF;
    else if ( (f == g) || (f == BDDT) ) return BDDT;
    else if ( g == BDDT ) return bddcopy(f);

    //////// read the result table ////////
    int h;
    int key = rthash( f, g, COFACT );
    Result* rt_key = rtable+key;
    if ( RTCMP(rt_key, f, g, COFACT) ) {
        rthit++;
        return bddcopy(rt_key->h);
    }

    //////// recursive computation ////////
    int f_ = rmN(f);
    int g_ = rmN(g);
    int flev = node[f_].lev;
    int glev = node[g_].lev;
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    if ( isN(f) ) { f0 = ~f0; f1 = ~f1; }
    int g0 = node[g_].edge0;
    int g1 = node[g_].edge1;
    if ( isN(g) ) { g0 = ~g0; g1 = ~g1; }
    if ( flev < glev ) {
        if ( g0 == BDDF ) h = bddcofact(f, g1);
        else if ( g1 == BDDF ) h = bddcofact(f, g0);
        else {
            //// restrict
            int g01 = ~bddand( ~g0, ~g1 );
            h = bddcofact(f, g01);
            bddfree(g01);
            // 	    //// constrain ( generalized cofactor )
            // 	    h = getnode(bddcofact(f, g0), bddcofact(f, g1), glev);
        }
    }
    else if ( flev == glev ) {
        if ( g0 == BDDF ) h = bddcofact(f1, g1);
        else if ( g1 == BDDF ) h = bddcofact(f0, g0);
        else {
            // 	    //// restrict and constrain
            // 	    h = getnode(bddcofact(f0, g0), bddcofact(f1, g1), flev);
            //// new heuristic: find function compatible with
            ////   [f0*g0, f0+!g0] and [f1*g1, f1+!g1] from f0 or f1
            ////   if not same as restrict and constrain
            int f0on = bddand(f0, g0);
            int f1on = bddand(f1, g1);
            int f0of = bddand(~f0, g0);
            int f1of = bddand(~f1, g1);
            int nf = BDDNULL, ng = BDDNULL;
            if ( !bddinter(f0on, f1of) && !bddinter(f0of, f1on) ) {
                int nfon = ~bddand( ~f0on, ~f1on );
                int nfof = ~bddand( ~f0of, ~f1of );
                if ( nfon == f0 || nfon == f1 ) nf = bddcopy(nfon);
                else if ( nfof == ~f0 || nfof == ~f1 ) nf = ~bddcopy(nfof);
                else nf = BDDNULL;
                if ( nf != BDDNULL ) ng = ~bddand( ~nfon, ~nfof );
                bddfree(nfon); bddfree(nfof);
            }
            if ( nf != BDDNULL ) {
                h = bddcofact(nf, ng);
                bddfree(nf); bddfree(ng);
            }
            else
                h = getnode(bddcofact(f0, g0), bddcofact(f1, g1), flev);
            bddfree(f0on); bddfree(f1on); bddfree(f0of); bddfree(f1of);
        }
    }	
    else
        h = getnode(bddcofact(f0, g), bddcofact(f1, g), flev);
    //////// write to the result table ////////
    RTWRITE(rtable+key, f, g, COFACT, h);
    return h;
}

//// compute ~x_var * f  +  x_var * g , where var is the level of x_var
//// and nodes of f and g of level >= var are up-shifted 1 level.
////
int bddshcompo(int f, int g, int var)
{
    //////// terminal cases ////////
    if ( f == g ) return bddlevshift(f, var, 1);
    int f_ = rmN(f);			//// f_ may be BDDF
    int g_ = rmN(g);			//// g_ may be BDDF
    int flev = node[f_].lev;		//// flev may be 0
    int glev = node[g_].lev;		//// glev may be 0
    if ( flev < var && glev < var )
        return getnode(bddcopy(f), bddcopy(g), var);

    //////// read the result table ////////
    int op_var = (var<<OPLOG) ^ SHCOMPO;
    int key = rthash(f, g, op_var);
    Result* rt_key = rtable+key;
    if ( RTCMP(rt_key, f, g, op_var) ) {
        rthit++;
        return bddcopy( rt_key->h );
    }

    //////// recursive computation ////////
    int f0=0, f1=0, g0=0, g1=0, h;
    if ( flev >= glev ) {
        f0 = node[f_].edge0;
        f1 = node[f_].edge1;
        if ( isN(f) ) { f0 = ~f0; f1 = ~f1; }
    }
    if ( flev <= glev ) {
        g0 = node[g_].edge0;
        g1 = node[g_].edge1;
        if ( isN(g) ) { g0 = ~g0; g1 = ~g1; }
    }
    if ( flev < glev )
        h = getnode(bddshcompo(f, g0, var), bddshcompo(f, g1, var), glev+1);
    else if ( flev > glev )
        h = getnode(bddshcompo(f0, g, var), bddshcompo(f1, g, var), flev+1);
    else //// flev == glev
        h = getnode(bddshcompo(f0, g0, var), bddshcompo(f1, g1, var), flev+1);

    //////// write to the result table ////////
    RTWRITE(rtable+key, f, g, op_var, h);
    return h;
}

/* bsup: varSet of before, asuplev: # of varSet of after */
int rec_compress(int f, int bsup, int asuplev)
{
    //////// terminal cases ////////
    int f_ = rmN(f);
    if ( f_ == BDDF ) return f;
    int flev = node[f_].lev;

    assert( bsup > 0 );
    int bsuplev = node[bsup].lev;
    if ( bsuplev == asuplev ) return (node[f_].inc(), f);
    assert( flev <= bsuplev );
    int bsup1 = node[bsup].edge0;
    if ( flev < bsuplev ) return rec_compress(f, bsup1, asuplev-1);

    //////// read the result table ////////
    int h;
    int key = rthash( f_, bsup, COMPRESS );
    Result* rt_key = rtable+key;
    if ( RTCMP(rt_key, f_, bsup, COMPRESS) ) {
        rthit++;
        h = rt_key->h;
        node[h].inc();	//// h must be positive
        if ( isN(f) ) h = ~h;
        return h;
    }

    //////// recursive computation ////////
    int h0 = rec_compress(node[f_].edge0, bsup1, asuplev-1);
    int h1 = rec_compress(node[f_].edge1, bsup1, asuplev-1);
    h = getnode(h0, h1, asuplev);
    //////// write to the result table ////////
    RTWRITE(rtable+key, f_, bsup, COMPRESS, h);
    if ( isN(f) ) h = ~h;
    return h;
}

bddid bddcompress(bddid f) {
    bddid f_levs = bddvarset(f);
    bddid result = rec_compress(f, f_levs, bddsize(f_levs));
    bddfree(f_levs);
    return result;
}

//////////////////////////////////////////////////////////////////

int zbddunion(int f, int g)
{
    //////// terminal cases ////////
    if ( f == ZBDDE ) return bddcopy(g);
    else if ( g == ZBDDE || f == g ) return bddcopy(f);
    else if ( f == ZBDDB ) return bddcopy( isN(g) ? g : ~g );
    else if ( g == ZBDDB || f == ~g )
        return bddcopy( isN(f) ? f : ~f );

    //////// read the result table ////////
    int f_ = rmN(f);
    int g_ = rmN(g);
    int flev = node[f_].lev;
    int glev = node[g_].lev;
    int h, swap=0;
    if ( (flev < glev) || ((flev == glev) && (f_ < g_)) ) {
        h = f; f = g; g = h;
        h = f_; f_ = g_; g_ = h;
        swap = 1;
    }
    int key = rthash( f_, g_, UNION );
    Result* rt_key = rtable+key;
    if ( RTCMP( rt_key, f_, g_, UNION ) ) {
        rthit++;
        h = rt_key->h;
        return ( isN(f) || isN(g) ) ? bddcopy( ~h ) : bddcopy(h);
    }

    //////// recursive computation ////////
    if ( swap ) {
        int tmp = flev; flev = glev; glev = tmp;
    }
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    if ( isN(f) ) { f0 = ~f0; }
    if ( flev > glev ) 
        h = getzbdd(zbddunion(f0,g), bddcopy(f1), flev);
    else { // flev == glev
        int g0 = node[g_].edge0;
        int g1 = node[g_].edge1;
        if ( isN(g) ) { g0 = ~g0; }
        h = getzbdd(zbddunion(f0,g0), zbddunion(f1,g1), flev);
    }
    //////// write to the result table ////////
    RTWRITE( rtable+key, f_, g_, UNION, ( isN(f) || isN(g) ) ? ~h : h );
    return h;
}

int zbddintersec(int f, int g)
{
    /*-------- terminal cases --------*/
    if ( f == ZBDDE || g == ZBDDE ) return ZBDDE;
    else if ( f == ZBDDB ) return isN(g) ? ZBDDB : ZBDDE;
    else if ( g == ZBDDB ) return isN(f) ? ZBDDB : ZBDDE;
    else if ( f == g ) return bddcopy(f);
    else if ( f == ~g ) return bddcopy( rmN(f) );

    /*-------- read the result table --------*/
    int f_ = rmN(f);
    int g_ = rmN(g);
    int flev = node[f_].lev;
    int glev = node[g_].lev;
    int h, swap=0;
    if ( (flev < glev) || ((flev == glev) && (f_ < g_)) ) {
        h = f; f = g; g = h;
        h = f_; f_ = g_; g_ = h;
        swap = 1;
    }
    int key = rthash(f_, g_, INTERSEC);
    Result* rt_key = rtable+key;
    if ( RTCMP( rt_key, f_, g_, INTERSEC ) ) {
        rthit++;
        h = rt_key->h;
        return ( isN(f) && isN(g) ) ? bddcopy( ~h ) : bddcopy(h);
    }
    /*-------- recursive computation --------*/
    if ( swap ) {
        int tmp = flev; flev = glev; glev = tmp;
    }
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    if ( isN(f) ) { f0 = ~f0; }
    if ( flev > glev ) 
        h = zbddintersec(f0,g);
    else { // flev == glev
        int g0 = node[g_].edge0;
        int g1 = node[g_].edge1;
        if ( isN(g) ) { g0 = ~g0; }
        h = getzbdd(zbddintersec(f0,g0), zbddintersec(f1,g1), flev);
    }
    /*-------- write to the result table --------*/
    RTWRITE( rtable+key, f_, g_, INTERSEC, ( isN(f) && isN(g) ) ? ~h : h );
    return h;
}

int zbdddiff(int f, int g)
{
    //////// terminal cases ////////
    if ( f == ZBDDE || f == g ) return ZBDDE;
    else if ( g == ZBDDE ) return bddcopy(f);
    else if ( f == ZBDDB ) return isN(g) ? ZBDDE : ZBDDB;
    else if ( g == ZBDDB ) return bddcopy( rmN(f) );
    else if ( f == ~g ) return isN(f) ? ZBDDB : ZBDDE;

    //////// read the result table ////////
    int f_ = rmN(f);
    int g_ = rmN(g);
    int flev = node[f_].lev;
    int glev = node[g_].lev;
    int h;
    int key = rthash( f_, g_, DIFF );
    Result* rt_key = rtable+key;
    if ( RTCMP( rt_key, f_, g_, DIFF ) ) {
        rthit++;
        h = rt_key->h;
        return bddcopy( (isN(f) && !isN(g)) ? ~h : h );
    }

    //////// recursive computation ////////
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    if ( isN(f) ) { f0 = ~f0; }
    if ( flev > glev ) 
        h = getzbdd(zbdddiff(f0,g), bddcopy(f1), flev);
    else { // flev == glev
        int g0 = node[g_].edge0;
        int g1 = node[g_].edge1;
        if ( isN(g) ) { g0 = ~g0; }

        if ( flev < glev )
            h = zbdddiff(f, g0);
        else
            h = getzbdd(zbdddiff(f0,g0), zbdddiff(f1,g1), flev);
    }
    //////// write to the result table ////////
    RTWRITE( rtable+key, f_, g_, DIFF, ( isN(f) && !isN(g) ) ? ~h : h );
    return h;
}

int zbddproduct(int f, int g)
{
    //////// terminal cases ////////
    if ( f == ZBDDE || g == ZBDDE ) return ZBDDE;
    else if ( f == ZBDDB ) return bddcopy(g);
    else if ( g == ZBDDB ) return bddcopy(f);

    //////// read the result table ////////
    int f_ = rmN(f);
    int g_ = rmN(g);
    int flev = node[f_].lev;
    int glev = node[g_].lev;
    int h, swap=0;
    if ( (flev < glev) || ((flev == glev) && (f_ < g_)) )
	{ h = f; f = g; g = h; swap = 1; }

    int key = rthash( f, g, PRODUCT );
    Result* rt_key = rtable+key;
    if ( RTCMP( rt_key, f, g, PRODUCT ) ) {
        rthit++;
        return bddcopy( rt_key->h );
    }

    /*-------- recursive computation --------*/
    if ( swap ) {
        h = f_; f_ = g_; g_ = h;
        int tmp = flev; flev = glev; glev = tmp;
    }
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    if ( isN(f) ) { f0 = ~f0; }
    if ( flev > glev ) 
        h = getzbdd(zbddproduct(f0,g), zbddproduct(f1,g), flev);
    else { // flev == glev
        int g0 = node[g_].edge0;
        int g1 = node[g_].edge1;
        if ( isN(g) ) { g0 = ~g0; }
        int h1a = zbddproduct(f0, g1);
        int h1b = zbddproduct(f1, g0);
        int h1ab = zbddunion(h1a, h1b);
        bddfree(h1a); bddfree(h1b);

        int h1c = zbddproduct(f1, g1);
        int h1 = zbddunion(h1ab, h1c);
        bddfree(h1ab); bddfree(h1c); 
	
        int h0 = zbddproduct(f0, g0);
        h = getzbdd(h0, h1, flev);
    }
    //////// write to the result table ////////
    RTWRITE( rtable+key, f, g, PRODUCT, h );
    return h;
}

int zbdddivide(int f, int g)  // f / g :  algebraic-division
{
    //////// terminal cases ////////
    if ( g == ZBDDB ) return bddcopy(f);
    else if ( f == g ) return ZBDDB;
    assert( g != ZBDDE );

    int f_ = rmN(f);
    if ( f_ == ZBDDE ) return ZBDDE; // f == ZBDDB or ZBDDE

    int flev = node[f_].lev;
    int g_ = rmN(g);
    int glev = node[g_].lev;
    if ( flev < glev ) return ZBDDE; // flev < glev : gにある変数がfにない

    //////// read the result table ////////
    int key = rthash( f, g, DIVIDE );
    Result* rt_key = rtable+key;
    if ( RTCMP( rt_key, f, g, DIVIDE ) ) {
        rthit++;
        return bddcopy( rt_key->h );
    }

    /*-------- recursive computation --------*/
    int h;
    int f0 = node[f_].edge0;
    if ( isN(f) ) { f0 = ~f0; }
    int f1 = node[f_].edge1;

    if ( flev > glev ) {
        int h0 = zbdddivide(f0, g);
        int h1 = zbdddivide(f1, g);
        h = getzbdd(h0, h1, flev);
    }
    else { // flev == glev
        int h0 = BDDNULL;
        int h1 = BDDNULL;
        int g0 = node[g_].edge0;
        if ( isN(g) ) { g0 = ~g0; }

        if ( g0 != ZBDDE ) {
            h0 = zbdddivide(f0, g0);
            if ( h0 == ZBDDE ) return ZBDDE;
        }
        int g1 = node[g_].edge1;
        if ( g1 != ZBDDE ) {
            h1 = zbdddivide(f1, g1);
            if ( h1 == ZBDDE ) {
                bddfree(h0); // h0を計算してしまったから
                return ZBDDE;
            }
        }

        if ( h0 == BDDNULL ) h = h1;
        else if ( h1 == BDDNULL ) h = h0;
        else {
            h = zbddintersec(h0, h1);
            bddfree(h0); bddfree(h1);
        }
    }

    //////// write to the result table ////////
    RTWRITE( rtable+key, f, g, DIVIDE, h );
    return h;
}
