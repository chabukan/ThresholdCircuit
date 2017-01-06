//////////////////////////////////////////////////////////////////
////////        BDD manipulator for decomposition         ////////
//////////////////////////////////////////////////////////////////

#include <memory.h>
#include "bddnode.h"
#include "bddop.h"

//////////////////////////////////////////////////////////////////

static int	check_decomp(int f, int n_nd);
static int	act_decomp_s(int f, int g_num, int *gg, int assign);
static int	act_decomp(int f, int n_nd, int g_num, int *gg, int assign);
static int	num_cutset(int f);
static void	findcutfuncs(int f, int assign);
static int	makesubfunc(int f, int bitposi, int assign);
static void	encode_heuristic(int f, int assign);
static void	encode_straight(int f, int assign);
static void	fill_cutfuncs(int* cutf_ass, int size);
static void	cut_decomp(int f, int n_nd, int g_num, int assign);

//////////////////////////////////////////////////////////////////

static int**	cutfuncs;
static int	cutsize;
static int	assignsize;
static int	cutfuncsptr;

static int	level;

static int	topvar;
static int	distopvar;
static int	ubound;

static char	env_encode = '1';

//////////////////////////////////////////////////////////////////

Bdd Bdd::decomp(int lev, int n_nd, int n_cut, Bdd *gg) const 
{
    int i;

    ::level = lev;
    ::topvar = this->top();
    ::distopvar = ::topvar - n_nd;
    ::ubound = n_cut;

    int g_num=1;
    while ( (1<<g_num) < n_cut ) g_num++;

    if ( check_decomp(edge, n_nd) == 0 ) return Bdd::null;

    // encode ‚Ì•û–@‚ðŒˆ‚ß‚é
    // 0: encode_straight, 1: encode_heuristic
    char* env = getenv("_BDD_DECOMP_ENCODE");
    if ( env ) {
	if ( env[0] == 's' ) ::env_encode = '0'; // straight
	else if ( env[0] == 'h' ) ::env_encode = '1'; // heuristic
    }

    // ŽÀÛ‚É decomp ‚·‚é
    ::cutsize = 1 << (g_num);
    ::assignsize = 1 << n_nd;
    cutfuncs = new int* [assignsize];
    for ( i=0; i<assignsize; i++ )
	cutfuncs[i] = (int*)calloc(cutsize, sizeof(int)); //// assign BDDNULL

    cut_decomp(edge, n_nd, g_num, 0);

    int* ggint = new int[g_num];
    Bdd result;
    result.edge = act_decomp(edge, n_nd, g_num, ggint, 0);
    for ( i=g_num; --i>=0; ) gg[i].edge = ggint[i];
    delete [] ggint;
    
    //// verification 
    Bdd tmpf = result;
    for ( i=1; i<=g_num; i++ )
	tmpf = tmpf.compose( ::topvar + i, gg[i-1]);
    assert(*this == tmpf);

    // memory free
    for ( i=0; i<assignsize; i++ )
	free(cutfuncs[i]);
    delete [] cutfuncs;

    return result;
}

int Bdd::decomp_check(int lev, int n_nd, int ubound) const
{
    ::level = lev;
    ::topvar = this->top();
    ::distopvar = ::topvar - n_nd;
    ::ubound = ubound;
    return check_decomp(edge, n_nd);
}

static int check_decomp(int f, int n_nd)
{
    if ( n_nd == 0 ) {
	int num = num_cutset(f);
	clearMark(f);
	return (num <= ::ubound) ? num : 0;
    }
    else {
	int ndvartop = ::distopvar+n_nd;
	int f0 = rstrtopup(f, ndvartop);
	int f0_num = check_decomp(f0, n_nd-1);
	if ( f0_num ) {
	    int f1 = rstrtopup(f, ~ndvartop);
	    int f1_num = check_decomp(f1, n_nd-1);
	    if ( f1_num == 0 ) return 0;
	    else return (f0_num >= f1_num) ? f0_num : f1_num;
	}
	else return 0;
    }
}

static int num_cutset(int f)
{
    int f_ = rmN(f);
    if ( isN(f) ) {
	if ( node[f_].mark & (1<<1) ) return 0;
	node[f_].mark |= (1<<1);
    }
    else {
	if ( node[f_].mark & 1 ) return 0;
	node[f_].mark |= 1;
    }
    if ( node[f_].lev <= ::level ) {
	return 1;
    }
    else { // flev > ::level
	int f0 = node[f_].edge0;
	int f1 = node[f_].edge1;
	if ( isN(f) ) { f0 = ~f0; f1 = ~f1; }
	return num_cutset(f0) + num_cutset(f1);
    }
}

static void cut_decomp(int f, int n_nd, int g_num, int assign)
{
    if ( n_nd == 0 ) {
	cutfuncsptr = 0;
	findcutfuncs(f, assign);
	clearMark(f);
	fill_cutfuncs(cutfuncs[assign], cutsize);

// 	printf("[%d] ", assign);
// 	for ( int j=0; j<cutsize; j++ ) 
// 	    printf("%6d ", cutfuncs[assign][j]);
// 	printf("\n");
    }
    else {
	int ndvartop = ::distopvar+n_nd;
	int f0 = rstrtopup(f, ndvartop);
	int f1 = rstrtopup(f, ~ndvartop);
	cut_decomp(f0, n_nd-1, g_num, 0 | assign);
	cut_decomp(f1, n_nd-1, g_num, (1<<(n_nd-1)) | assign);
    }
}

static void findcutfuncs(int f, int assign)
{
    int f_ = rmN(f);
    if ( isN(f) ) {
	if ( node[f_].mark & (1<<1) ) return;
	node[f_].mark |= (1<<1);
    }
    else {
	if ( node[f_].mark & 1 ) return;
	node[f_].mark |= 1;
    }
    int flev = node[f_].lev;
    if ( flev > ::level ) {
	int f0 = node[f_].edge0;
	int f1 = node[f_].edge1;
	if ( isN(f) ) { f0 = ~f0; f1 = ~f1; }
	findcutfuncs(f0, assign);
	findcutfuncs(f1, assign);
    }
    else {
	assert( cutfuncsptr < cutsize );

	switch( ::env_encode ) {
	case '0':
	    encode_straight(f, assign);
	    break;
	case '1':
	    encode_heuristic(f, assign);
	    break;
	}
	cutfuncsptr++;
    }
}

static void encode_heuristic(int f, int assign)
{
    int i, j;
    for ( i=0; i<assign; i++ ) for ( j=0; j<cutsize; j++ )
	if ( cutfuncs[i][j] == f ) {	//// there exist same func before
	    if ( cutfuncs[assign][j] == BDDNULL ) {
		//// here exists no func
		cutfuncs[assign][j] = f;
		return;
	    }
	    else {	//// here exist another func
		int old_has_relation = 0;
		int old_func = cutfuncs[assign][j];
		for ( int k=0; k<assign; k++ )
		    if ( cutfuncs[k][j] == old_func ) {
			old_has_relation = 1;
			break;
		    }
		if ( old_has_relation ) continue;

		//// use this place for f, another place for old_func
		cutfuncs[assign][j] = f;
		encode_heuristic(old_func, assign);
		return;
	    }
	}
    //// there exist no same func before
    for ( j=0; j<cutsize; j++ )
	if ( cutfuncs[assign][j] == BDDNULL ) {
	    cutfuncs[assign][j] = f;
	    return;
	}
    assert(0);
}

static void encode_straight(int f, int assign)
{
    cutfuncs[assign][cutfuncsptr] = f;
}

//// no effect for g\'s (subfuncs)
//// effect for projection
////
static void fill_cutfuncs(int* cutf_ass, int size)
{
    if ( size == 1 ) return;

    int harf = size>>1;
    int cannot = 0;
    int i;

    for ( i=0; i<harf; i++ )
	if ( (cutf_ass[i] != BDDNULL) &&
	     (cutf_ass[harf+i] != BDDNULL) &&
	     (cutf_ass[i] != cutf_ass[harf+i]) ) {
	    cannot = 1;
	    break;
	}

    if ( cannot == 0 ) for ( i=0; i<harf; i++ ) {
	if ( cutf_ass[i] == BDDNULL ) cutf_ass[i] = cutf_ass[harf+i];
	if ( cutf_ass[harf+i] == BDDNULL ) cutf_ass[harf+i] = cutf_ass[i];
    }

    fill_cutfuncs(cutf_ass, harf);
    fill_cutfuncs(cutf_ass + harf, harf);
    return;
}

static int act_decomp(int f, int n_nd, int g_num, int *gg, int assign)
{
    if ( n_nd == 0 ) return act_decomp_s(f, g_num, gg, assign);

    int ndvartop = ::distopvar+n_nd;
    int* g0 = new int[g_num];
    int* g1 = new int[g_num];
    int f0 = rstrtopup(f, ndvartop);
    int f1 = rstrtopup(f, ~ndvartop);
    int h0 = act_decomp(f0, n_nd-1, g_num, g0, 0 | assign);
    int h1 = act_decomp(f1, n_nd-1, g_num, g1, (1<<(n_nd-1)) | assign);
    
    for ( int i=0; i<g_num; i++ ) {
	gg[i] = bddvarite(ndvartop, g1[i], g0[i]);
	bddfree(g0[i]);
	bddfree(g1[i]);
    }
    int result = bddvarite(ndvartop, h1, h0);
    bddfree(h0);
    bddfree(h1);

    delete [] g1;
    delete [] g0;
    return result;
}

static int act_decomp_s(int f, int g_num, int *gg, int assign)
{
    int i, j;

//     printf("[%d] ", assign);
    for ( i=0; i<g_num; i++ ) {
	gg[i] = makesubfunc(f, i, assign);
// 	printf("%6d ", gg[i]);
    }
//     printf("\n");

    int* cutfuncs2 = cutfuncs[assign];
    //// result = cutfuncs2[0]; (projection)
    for ( j=0; j<cutsize; j++ )	bddcopy(cutfuncs2[j]);
    for ( i=1; i<=g_num; i++ ) for ( j=0; j < (1 << (g_num-i)); j++ )
	cutfuncs2[j] = getnode(cutfuncs2[(j<<1)], cutfuncs2[(j<<1)+1], 
			       ::topvar+i);
    return cutfuncs2[0];
}

static int makesubfunc(int f, int bitposi, int assign)
{
    //////// terminal cases ////////
    int f_ = rmN(f);
    int flev = node[f_].lev;	//// flev=0 when f_ == BDDF
    if ( flev <= ::level ) {
	for ( int i=0; i<cutsize; i++ )
	    if ( f == cutfuncs[assign][i] ) {
		if ( i & (1<<bitposi) ) return BDDT;
		else return BDDF;
	    }
	assert(0);
    }

    //////// recursive computation ////////
    int f0 = node[f_].edge0;
    int f1 = node[f_].edge1;
    if ( isN(f) ) { f0 = ~f0; f1 = ~f1; }
    int h0 = makesubfunc(f0, bitposi, assign);
    int h1 = makesubfunc(f1, bitposi, assign);
    return getnode(h0, h1, flev);
}
