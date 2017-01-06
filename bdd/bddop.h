#ifndef _bddop_h
#define _bddop_h

#include <stdio.h>
#include "bddc.h"
#include "arrayint.h"

#include "utility.h"

using namespace std;

class VarSymm;

class Bdd { // a wrapper class for bddid defined on bddc.h
public:
    static int	objnum;
    Bdd() : edge(BDDNULL) { ++objnum; }
    Bdd(bddid eg) : edge(eg) { ++objnum; } // 注意: bddポインタは吸収される
    Bdd(const Bdd& oprand) : edge(bddcopy(oprand.edge)) { ++objnum; }
    ~Bdd() { bddfree(edge); objnum--; }

    Bdd		operator = (const Bdd& oprand) {
	if ( *this != oprand ) {
	    bddfree(edge);
	    edge = bddcopy(oprand.edge);
	}
	return *this;
    }

    bddid	value() const { return edge; }
    bddid	getRep() const { return edge; }
    friend int	operator == (const Bdd& lhs, const Bdd& rhs)
	{ return (lhs.edge == rhs.edge); }
    friend int	operator != (const Bdd& lhs, const Bdd& rhs)
	{ return (lhs.edge != rhs.edge); }

    static const class Bdd	null;
    static const class Bdd	zero;
    static const class Bdd	one;

    static void	alloc(int init_p=16, int max_p=20) { bddalloc(init_p, max_p); }
    static void	status() { bddstatus(); }
    static void	gc() { bddgc(); }
    static int	used() { return bddused(); }

    static Bdd	var(int lev) { return Bdd( bddvar(lev) ); }

    Bdd		varSet() const;   // varSet()はZBDD表現．
    /* Bdd sup = f.varSet(); while ( sup != Bdd::empty )
	{ lev = sup.top(); sup = sup.subset0(lev); }     */

    int		top() const { return bddtop(edge); }
    int		size() const { return bddsize(edge); }
    int		size(int lev) const { return bddsize(edge, lev, lev); }
    int		size(int i, int j) const { return bddsize(edge, i, j); }

    Bdd		operator ~ () const { return Bdd( bddnot(edge) ); }

    inline friend Bdd	operator & (const Bdd& lhs, const Bdd& rhs);
    inline friend Bdd	operator | (const Bdd& lhs, const Bdd& rhs);
    inline friend Bdd	operator ^ (const Bdd& lhs, const Bdd& rhs);

    Bdd         smooth(int lev) const {	return Bdd( bddsmooth(edge, lev) ); }
    Bdd         consensus(int lev) const {return Bdd(bddconsensus(edge, lev));}

    Bdd		rstr0(int lev) const { return Bdd( bddrstr(edge,  lev) ); }
    Bdd		rstr1(int lev) const { return Bdd( bddrstr(edge, ~lev) ); }
    Bdd		rstrtop0() const { return Bdd( bddrstrtop0(edge) ); }
    Bdd		rstrtop1() const { return Bdd( bddrstrtop1(edge) ); }

    Bdd         cofact(const Bdd& care) const
	{ return Bdd( bddcofact(edge, care.edge) ); }
    Bdd		compose(int level, const Bdd& g) const
	{ return Bdd( bddcompose(edge, level, g.edge) ); }
    static Bdd	varIte(int lev, const Bdd& f1, const Bdd& f0)
	{ return Bdd( bddvarite(lev, f1.edge, f0.edge) ); }

    inline friend int	bddInter(const Bdd& lhs, const Bdd& rhs);
    inline friend int	operator <= (const Bdd& lhs, const Bdd& rhs);

    uint        dense() const { return bdddense(edge); }

    Bdd		compress() const { return Bdd( bddcompress(edge) ); }
    Bdd		levshift(int lev, int degree) const;
    Bdd		jumpud(int i, int j) const;
    Bdd		jumpup(int i, int j) const;

    static Bdd	readCache(int op, const Bdd& f, const Bdd& g) {
	return Bdd( bddreadcache(op, f.edge, g.edge) );
    }
    static void	writeCache(int op, const Bdd& f, const Bdd& g, const Bdd& h) {
	bddwritecache(op, f.edge, g.edge, h.edge);
    }

    // 二項演算の演算結果テーブル(binaryop.cc)の演算名
    enum { SOP_PRODUCT = 12, INTERSECTED = 13, EXPAND_CUBE = 14, ISOP = 15,
           IRREDUNDANT = 16 };

    Bdd		changeOrder(const vector<int>& permutation) const; // 変数順変更
    vector<int>	getPermuForCompress() const;
    		// compressをchangeOrderで行うためのpermutationを得る

    static Bdd	minDc(const Bdd& L, const Bdd& U);

    Bdd		decomp(int level, int n_nd, int n_cut, Bdd *g) const;
    int		decomp_check(int level, int n_nd, int ubound) const;
    void	printX() const; // BDDのグラフをX-windowに表示
    /* Bddの表示用関数は，他に ../ger-base/ger-base.h や ./cube.h にある．*/

    void	symAnaly(short* pairVar, VarSymm* symRel) const;
    VarSymm	sym_ij(int i, int j) const;
    VarSymm     svsym_ij(int i, int j) const;
    void	andExt(VarSymm* symRel) const;
    void	symm1_ext(int to) const;
    Arrayint	xorFactor() const;
    Arrayint	andFactor() const;

    /*------ ZBDD ------*/
    static const class Bdd	empty;
    static const class Bdd	base;

    Bdd		change(int lev) const { return Bdd( zbddchange(edge, lev) ); }
    Bdd		subset0(int lev) const { return Bdd( zbddsubset(edge, lev) ); }
    Bdd		subset1(int lev) const { return Bdd( zbddsubset(edge, ~lev)); }

    static Bdd		intersec(const Bdd& lhs, const Bdd& rhs);
    inline friend Bdd	operator + (const Bdd& lhs, const Bdd& rhs);
    inline friend Bdd	operator - (const Bdd& lhs, const Bdd& rhs);
    inline friend Bdd	operator * (const Bdd& lhs, const Bdd& rhs);
    inline friend Bdd	operator / (const Bdd& lhs, const Bdd& rhs);

    int		card() const { return zbddcard(edge); }
    int		card(int lev) const { return zbddcard(edge,lev); }
    int		nCube() const { return zbddcard(edge); }
    int		nCube(int lev) const { return zbddcard(edge,lev); }
    int		multi() const { return zbddmulti(edge); }
    int		multi(int lev) const { return zbddmulti(edge,lev); }
    int		lit() const { return zbddlit(edge); }
    int		nLit() const { return zbddlit(edge); }
    void	litCount(uint* literals) const
	        { zbddlitcount(edge, literals); }
    void	litCount(uint* literals, int max) const
                { zbddlitcount(edge, literals, max); }
    void	multiLit(char* literals) const
	        { zbddmultilit(edge, literals); }
    Bdd         dupLit() const;

    Bdd         commonLit() const { return Bdd( zbddcommonlit(edge) ); }
    Bdd         sum2product() const { return Bdd( zbddsum2product(edge) ); }
    Bdd         delLitCube() const { return Bdd( zbddDelLitCube(edge) ); }

    Bdd		sop2func() const { return Bdd( zbddsop2func(edge) ); }
    Bdd         getAelement() const;

    Bdd		zjumpup(int i, int j) const;
    Bdd		zlevshift(int lev, int degree) const;

private:
    bddid	edge;
};

inline int bddHash(const Bdd& key) { return key.getRep(); }

//////////////////////////////////////////////////////////////////

inline Bdd Bdd::varSet() const {
    if ( *this == Bdd::null ) return *this;
    return Bdd( bddvarset(edge) );
}

inline Bdd operator & (const Bdd& lhs, const Bdd& rhs) {
    return Bdd( bddand(lhs.edge, rhs.edge) );
}
inline Bdd operator | (const Bdd& lhs, const Bdd& rhs) {
    return Bdd( bddor(lhs.edge, rhs.edge) );
}
inline Bdd operator ^ (const Bdd& lhs, const Bdd& rhs) {
    return Bdd( bddxor(lhs.edge, rhs.edge) );
}
inline int operator <= (const Bdd& lhs, const Bdd& rhs) {
    return !bddinter(lhs.edge, ~(rhs.edge));
}

inline Bdd Bdd::jumpup(int i, int j) const {
    assert(i<=j); return jumpud(i, j);
}
inline Bdd Bdd::jumpud(int i, int j) const {
    assert( (0 < i) && (i <= MAXLEV) && (0 < j) && (j <= MAXLEV) );
    return Bdd( bddjumpud(edge, i, j) );
}
inline Bdd Bdd::levshift(int lev, int degree) const {
    assert( (0 < lev) && (lev <= MAXLEV) &&
	    (-MAXLEV < degree) && (degree <= MAXLEV) );
    return Bdd( bddlevshift(edge, lev, degree) );
}

inline int bddInter(const Bdd& lhs, const Bdd& rhs) {
    return bddinter(lhs.edge, rhs.edge);
}

//////////////////////////////////////////////////////////////////

inline Bdd Bdd::intersec(const Bdd& lhs, const Bdd& rhs) {
    return Bdd( zbddintersec(lhs.edge, rhs.edge) );
}
inline Bdd operator + (const Bdd& lhs, const Bdd& rhs) {
    return Bdd( zbddunion(lhs.edge, rhs.edge) );
}
inline Bdd operator - (const Bdd& lhs, const Bdd& rhs) {
    return Bdd( zbdddiff(lhs.edge, rhs.edge) );
}
inline Bdd operator * (const Bdd& lhs, const Bdd& rhs) {
    return Bdd( zbddproduct(lhs.edge, rhs.edge) );
}
inline Bdd operator / (const Bdd& lhs, const Bdd& rhs) {
    return Bdd( zbdddivide(lhs.edge, rhs.edge) );
}

inline Bdd Bdd::zlevshift(int lev, int degree) const {
    assert( (0 < lev) && (lev <= MAXLEV) &&
            (-MAXLEV < degree) && (degree <= MAXLEV) );
    return Bdd( zbddlevshift(edge, lev, degree) );
}
inline Bdd Bdd::zjumpup(int i, int j) const {
    return Bdd( zbddjumpup(edge, i, j) );
}

#endif
