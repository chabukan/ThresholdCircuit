#include "bddnode.h"
#include "bddop.h"
#include "ordbdd.h"
#include "isf.h"
#include "arrayint.h"
#include "Array.H"
#include <set>

//////////////////////////////////////////////////////////////////


void IsfLU::elimVar(int var) {
    Bdd L = getL();
    L = L.smooth(var);
    Bdd U = getU();
    U = U.consensus(var);
    *this = IsfLU(L, U);
}
void IsfLU::elimLit0(int var) {
    Bdd L = getL();
    Bdd U = getU();
    Bdd L0 = L.rstr0(var);
    Bdd U1 = U.rstr1(var);
    L = L | (L0 &  Bdd::var(var));
    U = U & (U1 |  Bdd::var(var));
    *this = IsfLU(L, U);
}
void IsfLU::elimLit1(int var) {
    Bdd L = getL();
    Bdd U = getU();
    Bdd L1 = L.rstr1(var);
    Bdd U0 = U.rstr0(var);
    L = L | (L1 & ~Bdd::var(var));
    U = U & (U0 | ~Bdd::var(var));
    *this = IsfLU(L, U);
}

//  void IsfLU::forceVarSet(const Bdd& varset) // varsetで表される変数集合にする
//  {
//      Bdd supDiff = varSet() - varset;
//      while ( supDiff != Bdd::empty ) {
//          int var = supDiff.top();
//          smooth(var);
//          supDiff = supDiff.subset0(var);
//      }
//  }

void IsfLU::forceVarSet(const Arrayint& varset) // varsetで表される変数集合に
{
    Bdd sup = varSet();
    int top;
    int size = varset.size();
    for ( int i = 0; i < size; i++ ) {
	int var = varset[i];
	while ( sup != Bdd::empty ) {
	    top = sup.top();
	    if ( top > var ) {
		elimVar(top); // varsetに無くてsupにある変数
		sup = sup.subset0(top);
	    }
	    else if ( top == var ) {
		sup = sup.subset0(top);
		break;
	    }
	    else { // top < var, varsetにある変数がsupにない場合
		fprintf(stderr,
		"forceVarSet(), var %d in varset is not in sup\n", var);
		break;
	    }
	}
    }
    while ( sup != Bdd::empty ) {
	top = sup.top();
	elimVar(top);
	sup = sup.subset0(top);
    }
}

// litsetで表されるリテラル集合に
void IsfLU::forceLitSet(const Arrayint& litset)
{
    std::set<int> lit0;
    std::set<int> lit1;
    for ( int i = litset.size(); --i>=0; ) {
	int lit = litset[i];
	int var = (lit+1)>>1;
	if ( lit & 1 ) lit0.insert(var);
	else lit1.insert(var);
    }
//      fprintf(stderr, "lit0: ");
//      for ( set<int>::const_iterator p = lit0.begin();
//  	  p != lit0.end(); p++ ) {
//  	fprintf(stderr, "%d ", *p);
//      }
//      fprintf(stderr, "\n");
//      fprintf(stderr, "lit1: ");
//      for ( set<int>::const_iterator p = lit1.begin();
//  	  p != lit1.end(); p++ ) {
//  	fprintf(stderr, "%d ", *p);
//      }
//      fprintf(stderr, "\n");

    Bdd sup = varSet();
    while ( sup != Bdd::empty ) {
	int top = sup.top();

	bool no0 = (lit0.find(top) == lit0.end());
	bool no1 = (lit1.find(top) == lit1.end());
	if ( no0 && no1 ) elimVar(top);
	else if ( no0 ) elimLit0(top);
	else if ( no1 ) elimLit1(top);

	sup = sup.subset0(top);
    }
}

int IsfLU::sameVarSet(const Bdd& f)
{
    if ( !(getL() <= f) ) {
        abort();
        return -1;
    }
    else if ( !(f <= getU()) ) {
        abort();
        return -1;
    }

    Bdd supf = f.varSet();
    Bdd supfD = varSet();

    Bdd supDiff = supfD - supf;
    if ( supf + supDiff != supfD ) return 0;

    IsfLU newfD = *this;
    while ( supDiff != Bdd::empty ) {
        int var = supDiff.top();
        newfD.elimVar(var);
        supDiff = supDiff.subset0(var);
    }

    if ( !(newfD.getL() <= f) ) {
        abort();
        return -1;
    }
    else if ( !(f <= newfD.getU()) ) {
        abort();
        return -1;
    }

    *this = newfD;
    return 1;
}

Sop IsfLU::getBetterSop(int& polarity) {
    Sop p_sop = (*this).makeISOP();
    Sop n_sop = (~(*this)).makeISOP();
    if ( p_sop.nLit() <= n_sop.nLit() ) {
	polarity = 1;
	return p_sop;
    }
    else {
	polarity = 0;
	return n_sop;
    }
}

//////////////////////////////////////////////////////////////////


struct Interv {		//// interval [fL, fU]
    int L;
    int U;
    	Interv() : L(BDDNULL), U(BDDNULL) {}
    	Interv(int LL, int UU) : L(LL), U(UU) {}
    	Interv(const Interv& obj) : L(obj.L), U(obj.U) {}
    	~Interv() {}
    
    int operator == (const Interv obj) {
	return (L == obj.L) && (U == obj.U);
    }
    Interv elim_var(int var);
    Interv Icopy() { bddcopy(L); bddcopy(U); return *this; }
    Interv Ifree() { bddfree(L); bddfree(U); return *this; }
    int top_min() { 
	int Ltop = node[rmN(L)].lev;
	int Utop = node[rmN(U)].lev;
	return ( Ltop < Utop ) ? Ltop : Utop;
    }
    int top_max() { 
	int Ltop = node[rmN(L)].lev;
	int Utop = node[rmN(U)].lev;
	return ( Ltop > Utop ) ? Ltop : Utop;
    }
    Interv rstrtopup(int lev) {
	return Interv( ::rstrtopup(L, lev), ::rstrtopup(U, lev) );
    }

friend int is_compati(Interv f, Interv g, int lev);
friend Interv get_compati(Interv f, Interv g);
};

typedef Array< Arrayint > graph_t;

// #include "Array.C"
// template class Array< Interv >;
// template class Array< Arrayint >;

//// for minimize by dont care
static int	sub_interval(Array<Interv>& ff, Array<Interv>& gg, int maxlev);
static void	bddinterval(Array<Interv>& ff, Arrayint& gg);

//// for clique
static void	min_clique_cover(graph_t& original);
static int	max_clique(graph_t& graph);

//////////////////////////////////////////////////////////////////
//	minimize by dont care
//////////////////////////////////////////////////////////////////

Bdd IsfLU::minDc() const
{
    return Bdd::minDc(getL(), getU());
}

Bdd Bdd::minDc(const Bdd& L, const Bdd& U)
{
    if ( L == U ) return L;

    Array<Interv> ff;
    Arrayint gg;
    Interv intf = Interv(L.value(), U.value());
    ff.add(intf);
    bddinterval(ff, gg);

    Bdd result;
    result.edge = gg[0];

//     if ( *this != result ) 
// 	printf("minDc: (%d or %d) -> %d\n",
// 	       rstrtop1().size(), rstrtop0().size(), result.size() );
    return result;
}

//// share counter policy:
//// At first gg is empty, but gg have share counter when return.
////
static void bddinterval(Array<Interv>& ff, Arrayint& gg)
{
    int		i, j;
    int		n = ff.size();
    gg.reserve(n);

    for ( i=0; i<n; i++ ) assert( !bddinter( ff[i].L, ~ff[i].U ) );

    int maxlev = 0;
    for ( i=0; i<n; i++ )
	if ( maxlev < ff[i].top_max() ) maxlev = ff[i].top_max();
    
//     printf("ff[%d] ", maxlev);
//     for ( i=0; i<n; i++ ) 
// 	printf("[%d(%d), %d(%d)] ", ff[i].L, node[rmN(ff[i].L)].lev,
// 	       			    ff[i].U, node[rmN(ff[i].U)].lev);
//     printf("\n");

    //////// sub_interval ////////
    Array<Interv> hh;
    int change_flag = sub_interval(ff, hh, maxlev);

    int exist_zvar=0;
    for ( i=0; i<n; i++ ) if ( hh[i].L != hh[i].U ) {
	exist_zvar = 1;
	break;
    }
    if ( exist_zvar == 0 ) {
	assert(change_flag);
	for ( i=0; i<n; i++ ) { gg[i] = hh[i].L; bddfree(hh[i].U); }
	return;
    }

    Array<Interv> ff1;
    for ( i=0; i<n; i++ ) {
	Interv g0 = hh[i].rstrtopup(maxlev);
	if ( !ff1.contain(g0) ) ff1.add(g0);
	Interv g1 = hh[i].rstrtopup(~maxlev);
	if ( !ff1.contain(g1) ) ff1.add(g1);
    }

    //////// bddinterval ////////
    Arrayint gg1;
    bddinterval(ff1, gg1);

    int n_ff1 = ff1.size();
    for ( i=0; i<n; i++ ) {
	Interv h0 = hh[i].rstrtopup(maxlev);
	for ( j=0; j<n_ff1; j++ ) if ( ff1[j] == h0 ) break;
	int g0 = gg1[j];
	Interv h1 = hh[i].rstrtopup(~maxlev);
	for ( j=0; j<n_ff1; j++ ) if ( ff1[j] == h1 ) break;
	int g1 = gg1[j];
	gg[i] = getnode(bddcopy(g0), bddcopy(g1), maxlev);
    }

    if ( change_flag ) for ( i=0; i<n; i++ ) hh[i].Ifree();
    for ( i=0; i<n_ff1; i++ ) bddfree(gg1[i]);

//     printf("gg[%d] ", maxlev);
//     for ( i=0; i<n; i++ ) printf("%d ", gg[i]);
//     printf("\n");
}

//// share counter policy:
//// When change: ( return 1 )
////	At first gg is empty, but gg have share counter when return.
//// When no change: ( return 0 )
////	At first gg is empty, and gg have no share counter when return.
////
int sub_interval(Array<Interv>& ff, Array<Interv>& gg, int maxlev)
{
    int		i, j;
    int		n = ff.size();
    graph_t	original(n);
    gg.reserve(n);
    for ( i=0; i<n; i++ ) gg[i] = Interv();

    int exist_change = 0;
    for ( i=0; i<n; i++ ) if ( ff[i].top_max() == maxlev ) {
	Interv tmpif = ff[i].elim_var(maxlev);
	if ( !(tmpif == ff[i]) ) {
	    gg[i] = tmpif;
	    exist_change = 1;
	}
	else tmpif.Ifree();
    }

    for ( i=0; i<n; i++ ) for ( j=i+1; j<n; j++ )
	if ( gg[i] == Interv() && gg[j] == Interv() &&
	     is_compati(ff[i], ff[j], maxlev) ) {
	    original[i].add(j);
	    original[j].add(i);
	    exist_change = 1;
	}
    if ( exist_change == 0 ) { gg = ff; return 0; }

    min_clique_cover(original);

    for ( i=0; i<n; i++ ) {
	int size_clique = original[i].size();
	if ( size_clique ) {
	    Interv common = ff[i].Icopy();
	    for ( j=0; j<size_clique; j++ ) {
		int index = original[i][j];

		Interv tmpI = get_compati(common, ff[index]);
		common.Ifree();
		common = tmpI;

		original[index].clear();
	    }
	    for ( j=0; j<size_clique; j++ ) {
		int index = original[i][j];
		assert(gg[index] == Interv());
		gg[index] = common.Icopy();
	    }
	    assert(gg[i] == Interv());
	    gg[i] = common;	//// absorb share counter from getnode
	}
    }
    for ( i=0; i<n; i++ )
	if ( gg[i] == Interv() ) gg[i] = ff[i].Icopy();

//     for ( i=0; i<n; i++ )
// 	printf("[%d(%d), %d(%d)] ", gg[i].L, node[rmN(gg[i].L)].lev,
// 	       			    gg[i].U, node[rmN(gg[i].U)].lev);
//     printf("\n");
    return 1;
}

int is_compati(Interv f, Interv g, int lev)
{
    if ( f.top_min() != lev ) return 0;
    if ( g.top_min() != lev ) return 0;
    return !bddinter( f.L, ~g.U ) && !bddinter( g.L, ~f.U );
}

Interv get_compati(Interv f, Interv g)
{
    Interv h;
    h.L = bddor(f.L, g.L);
    h.U = bddand(f.U, g.U);
    assert( !bddinter(h.L, ~h.U) );
    return h;
}

Interv Interv::elim_var(int var)
{
    Interv h;
    h.L = bddsmooth(L, var);
    h.U = ~bddsmooth(~U, var);
    if ( !bddinter(h.L, ~h.U) ) {
	return h;
    }
    else {
	h.Ifree();
	return (*this).Icopy();
    }
}

//////////////////////////////////////////////////////////////////
//	clique
//////////////////////////////////////////////////////////////////

void min_clique_cover(graph_t& original)
{
    int		i, j;
    int		n = original.size();
    graph_t	for_max;
    graph_t	result(n);

    while ( 1 ) {
	//// find max clique
	if ( max_clique( for_max = original ) == 0 ) break;

// 	for ( i=0; i<n; i++ ) {
// 	    printf("[%d] ", i);
// 	    for ( j=0; j<for_max[i].size(); j++ )
// 		printf("%d ", for_max[i][j]);
// 	    printf("\n");
// 	}

	for ( i=0; i<n; i++ ) if ( for_max[i].size() ) {
	    //// clear max_clique from original
	    original[i].clear();
	    for ( j=0; j<n; j++ )
		if ( original[j].contain(i) ) original[j].del(i);
	    //// add max_clique to result
	    assert( result[i].size() == 0 );
	    result[i] = for_max[i];
	}
    }
    original = result;
    return;
}

static int	max_cl_deg;	//// max size of clique already found
static int	no_other_edge;	//// num of nodes that has no other edge
static graph_t	init_graph;	//// initial graph
static graph_t	max_cl;		//// max clique already found

static void	rec_clique(graph_t& graph, int concern);

int max_clique(graph_t& graph)
{
    int i;
    int n = graph.size();

    //// max degree of graph
    int max_deg=0;
    for ( i=0; i<n; i++ ) if ( max_deg < graph[i].size() )
	max_deg = graph[i].size();
    if ( max_deg == 0 ) return 0;

    ::max_cl_deg = 0;
    ::no_other_edge = -1;
    ::max_cl.clear();
    ::init_graph = graph;

    //// recursive computation
    for ( int sz = max_deg; sz > ::max_cl_deg; sz-- ) {
	for ( i=0; i<n; i++ ) if ( sz == graph[i].size() )
	    rec_clique(graph, i);
    }
    graph = max_cl;
    return 1;
}

static void rec_clique(graph_t& b_graph, int concern)
{
    int i, j;
    int n = b_graph.size();

    //// making graph whose nodes have edges to node_{concern}
    graph_t	graph(n);
    graph[concern] = b_graph[concern];
    for ( i=0; i<n; i++ ) if ( b_graph[concern].contain(i) ) {
	graph[i] = b_graph[i];
	for ( j=0; j<n; j++ ) 
	    if ( concern != j && !graph[concern].contain(j) &&
		 graph[i].contain(j) )
		graph[i].del(j);
    }

    //// max degree of graph
    int max_deg = graph[concern].size();
    assert( max_deg );

    //// if there exists the only one max clique
    int n_has_max=0;
    for ( i=0; i<n; i++ ) if ( max_deg == graph[i].size() )
	n_has_max++;
    if ( max_deg+1 == n_has_max ) {
	int tmp=0;
	for ( i=0; i<n; i++ )
	    if ( max_deg == graph[i].size() &&
		 max_deg == init_graph[i].size() ) tmp++;
	if ( ::no_other_edge < tmp ) {
	    ::no_other_edge = tmp;
	    ::max_cl = graph;
	    ::max_cl_deg = max_deg;
// 	    printf("rec_clique(graph, %d), max_cl_deg:%d no_other_edge:%d\n",
// 		   concern, ::max_cl_deg, ::no_other_edge);
	}
    }

    //// recursive computation
    for ( int sz = max_deg-1; sz > ::max_cl_deg; sz-- ) {
	for ( i=0; i<n; i++ ) if ( sz == graph[i].size() )
	    rec_clique(graph, i);
    }
}


//////////////////////////////////////////////////////////////////
//	support minimization
//////////////////////////////////////////////////////////////////

/*
int	min_support(Bdd& f, int max_sup);

依存変数最小化(Support Minimization)を行う．

Bdd& f:
	対象となる関数．結果はこの変数に収められる．top()の変数はzvar
int max_sup:
	入力数がmax_sup以下になれば成功，これを超えれば失敗とする．
int:
	依存変数最小化が失敗した場合は0を，成功した場合は1を返す．
*/

static  int	min_support(Bdd& f, int max_sup);

// #define DBG1
#ifdef DBG1
#define DEBUG1(statement) statement
#else
#define DEBUG1(statement)
#endif

static int	top_var;
static int	n_sup;
static Bdd*	minf;

// max_sup: 変数の数(zvarを除く)がmax_sup以下になれば成功．
int IsfLU::min_support(int max_sup)
{
    Bdd L = getL();
    Bdd U = getU();

    if ( L == U ) return 0;

    Bdd func = Bdd::varIte( top()+1, L, ~U );

    int suc = ::min_support(func, max_sup);

    if ( suc ) {
	*this = IsfLU( func.rstrtop1(), ~func.rstrtop0() );
    }
    return suc;
}

static void rec_find(Bdd& f, int removing, int n_remained);

// func: 対象となる関数．結果はこの変数に収められる．top()の変数はzvar
// max_sup: 変数の数(zvarを除く)がmax_sup以下になれば成功．
int min_support(Bdd& func, int max_sup)
{
    if ( func == Bdd::null ) return 0;

    OrdBdd ford(func);
    ford.compress();

    ::minf = new Bdd;
    ::n_sup = max_sup+1;
    ::top_var = ford.getFunc().top();	// この関数 func に割り当てられた変数
				// top の変数は必ず残る
    
    DEBUG1( printf("min_support() max_sup=%d, N=%d\n",
		   max_sup, ::top_var-1); );

    rec_find(ford.getFunc(), ::top_var-1, ::top_var-1);

    if ( ::n_sup <= max_sup ) {
	ford.setFunc( *(::minf) );
	assert(ford.getFunc().rstrtop0().varSet() == 
	       ford.getFunc().rstrtop1().varSet());
	ford.normalize();
	func = ford.getFunc();
	delete ::minf;
	return 1;
    }
    else {
	delete ::minf;
	return 0;
    }
}

//// check the elimination of a variable recursively from the top
//// variable to the bottom variable.
static void rec_find(Bdd& f, int removing, int n_remained)
{
    //// terminal case
    if ( removing <= 0 ) return;
    //// pruning by lower bound
    if ( n_remained - removing >= ::n_sup ) return;

    //// eliminate the removing
    Bdd newf = f.smooth(removing);

    //// valid
    if ( !bddInter( newf.rstr0(::top_var), newf.rstr1(::top_var) ) ) {
	if ( n_remained-1 < ::n_sup ) {
	    ::n_sup = n_remained-1;
	    *(::minf) = newf;
// 	    printf("in rec_find() ::n_sup=%d, removing=%d\n",
// 		   ::n_sup, removing);
	}
	rec_find(newf, removing-1, n_remained-1);
    }
    //// not eliminate the removing
    rec_find(f, removing-1, n_remained);
}

//////////////////////////////////////////////////////////////////

// 変数の展開順を指定した生成アルゴリズム
Sop IsfLU::makeISOPorder(const Arrayint& order, int idx)
{
    Bdd L = getL(); 
    Bdd U = getU(); 
    //------ terminal cases ------
    if( L == Bdd::zero ) return Sop::zero;
    else if( U == Bdd::one ) return Sop::one;

    //-------- recursive computation --------
    int level = order[idx];

    Bdd L0 = L.rstr0(level);
    Bdd L1 = L.rstr1(level);
    Bdd U0 = U.rstr0(level);
    Bdd U1 = U.rstr1(level);
    if ( (L0 == L1) && (U0 == U1) ) { 
        return makeISOPorder(order, idx+1);
    }
    else {
	IsfLU f00( (L0 & ~U1) ,U0);
	IsfLU f11( (L1 & ~U0) ,U1);
	Sop result_f0  = f00.makeISOPorder(order, idx+1); // isop0に相当
	Sop result_f1  = f11.makeISOPorder(order, idx+1);

	Bdd g0 = result_f0.getFunc(0);
	Bdd g1 = result_f1.getFunc(0);
	Bdd f000L = L0 & ~g0;
	Bdd f111L = L1 & ~g1;
	IsfLU isopAlpha( ((f000L & U1) | (U0 & f111L)), (U0 & U1)); 
	Sop result_fr = isopAlpha.makeISOPorder(order, idx+1); 

	return result_f0.and0(level) + result_f1.and1(level) + result_fr;
    }

//      IsfLU f0(rstr0(level));
//      IsfLU f1(rstr1(level));
//      if ( f0 != f1 ) {
//  	Bdd f0L = f0.getL(); 
//  	Bdd f0U = f0.getU(); 
//  	Bdd f1L = f1.getL(); 
//  	Bdd f1U = f1.getU(); 

//  	IsfLU f00( (f0L & ~f1U) ,f0U);
//  	IsfLU f11( (f1L & ~f0U) ,f1U);
//  	Sop result_f0  = f00.makeISOPorder(order, idx+1); // isop0に相当
//  	Sop result_f1  = f11.makeISOPorder(order, idx+1);

//  	Bdd g0 = (result_f0).getFunc(0);
//  	Bdd g1 = (result_f1).getFunc(0);
//  	Bdd f000L = f0L & ~g0;
//  	Bdd f111L = f1L & ~g1;
//  	IsfLU isopAlpha( ((f000L & f1U) | (f0U & f111L))  , (f0U & f1U)); 
//  	Sop result_fr = isopAlpha.makeISOPorder(order, idx+1); 

//  	return result_f0.and0(level) + result_f1.and1(level) + result_fr;
//      }
//      else {
//          return makeISOPorder(order, idx+1);
//      }
}
