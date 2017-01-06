#include <assert.h>
#include "ordbdd.h"
#include "varsymm.h"

// #define DBG1
#ifdef DBG1
#define DEBUG1(statement) statement
#else
#define DEBUG1(statement)
#endif

//////////////////////////////////////////////////////////////////
//	OrdBdd
//////////////////////////////////////////////////////////////////

OrdBdd::OrdBdd(Bdd g) : f(g) {
    int n = f.top();
    lev2idx = new int[n+1];
    lev2idx[0] = n;
    for ( int i=1; i<=n; i++ ) lev2idx[i] = i;
}

OrdBdd::OrdBdd(Bdd g, int* l2i) : f(g) {
    int n = f.top();
    lev2idx = new int[n+1];
    lev2idx[0] = n;
    for ( int i=1; i<=n; i++ ) lev2idx[i] = l2i[i];
}

static void replace_lev2idx(int*& oldo, int* newo);

OrdBdd& OrdBdd::operator = (const OrdBdd& op) {
    f = op.f;
    replace_lev2idx(lev2idx, op.lev2idx);
    return (*this);
}

static void replace_lev2idx(int*& oldo, int* newo) {
    if ( newo ) {
	if ( oldo ) {
	    if ( oldo[0] != newo[0] ) {
		delete [] oldo;
		oldo = new int[newo[0] + 1];
	    }
	}
	else {
	    oldo = new int[newo[0] + 1];
	}
	memcpy(oldo, newo, sizeof(int)*(newo[0]+1));
    }
    else {
	if ( oldo ) {
	    delete [] oldo;
	    oldo = 0;
	}
    }
}

void OrdBdd::jumpup(int i, int j) {
    if ( i != j ) {
	f = f.jumpud(i, j);
	int tmplev2idx = lev2idx[i];
	for ( int k=i; k<j; k++ ) lev2idx[k] = lev2idx[k+1];
	lev2idx[j] = tmplev2idx;
    }
}

void OrdBdd::jumpdown(int i, int j) {
    if ( i != j ) {
	f = f.jumpud(i, j);
	int tmplev2idx = lev2idx[i];
	for ( int k=i; k>j; k-- ) lev2idx[k] = lev2idx[k-1];
	lev2idx[j] = tmplev2idx;
    }
}

void OrdBdd::normalize()
{
    int max_var=0;
    int n_var = lev2idx[0];
    int i;
    for ( i=1; i<=n_var; i++ )
	if ( lev2idx[i] > max_var ) max_var = lev2idx[i];

    int* newlev2idx = new int[max_var+1];
    memcpy(newlev2idx, lev2idx, (n_var+1)*sizeof(int));
    for ( i=n_var+1; i<=max_var; i++ ) newlev2idx[i] = 0;
    delete [] lev2idx;
    lev2idx = newlev2idx;
    
    /* idxの大きいものから，lev=idxとなるようにしていく */
    for ( int idx=max_var; idx>=1; idx-- ) {
	for ( int lev=1; lev<=n_var; lev++ ) if ( lev2idx[lev] == idx ) {
	    jumpup(lev, idx);
	    break;
	}
    }

    lev2idx[0] = max_var;
    for ( i=1; i<=max_var; i++ ) {
	if ( lev2idx[i] ) assert( lev2idx[i] == i );
	else lev2idx[i] = i;
    }
}

void OrdBdd::compress()
{
    int i, j;
    
    assert( f.top() <= lev2idx[0] );

    int* var_set = new int[lev2idx[0]+1];
    for ( i=1; i<=lev2idx[0]; i++ ) var_set[i] = 0;
    
    Bdd f_levs = f.varSet();
    Bdd tmpf = f_levs;
    while ( tmpf != Bdd::empty ) {
	i = tmpf.top();
	DEBUG1( printf("%d ", i); );
	var_set[i]++;
	tmpf = tmpf.subset0(i);
    }
    DEBUG1( printf("\n"); );
    for ( j=1, i=1; i<=lev2idx[0]; i++ )
	if ( var_set[i] ) lev2idx[j++] = lev2idx[i];
    lev2idx[0] = --j;
    f = f.compress();
    assert( f.top() == lev2idx[0] );
    delete [] var_set;
}

void OrdBdd::levshift(int lev, int degree)
{
    assert( lev > 0 );
    int n = lev2idx[0];
    int* newlev2idx = new int[n + degree + 1];
    for ( int i = 1; i < lev; i++ ) newlev2idx[i] = lev2idx[i];
    for ( int i = lev; i < lev+degree; i++ ) newlev2idx[i] = -1;
    for ( int i = lev; i <= n; i++ ) newlev2idx[i+degree] = lev2idx[i];
    newlev2idx[0] = n+degree;

    delete lev2idx;
    lev2idx = newlev2idx;

    f = f.levshift(lev, degree);
}

int OrdBdd::index2level(int idx)
{
    int i;
    for ( i=lev2idx[0]; i>0; i-- ) if ( lev2idx[i] == idx ) break;
    return i;
}

void OrdBdd::symReorder(short* pairVar, VarSymm* symRel)
{		// 対称な変数を隣り合わせにする
    int i, k, n = nVar();

    //------ pairVar を並べ替え，対称な変数がgroupになるように
    for ( i=n; i>=1; i-- ) {
        int j = pairVar[i];
        if ( j == 0 || j == i-1 ) continue;     //// ない or となり

        jumpup(j, i-1);
        //// jumpup に伴って pairVar を sort
        int tmp = pairVar[j];
        VarSymm tmp2 = symRel[j];
        for ( k=j+1; k<=i-1; k++ ) {
            pairVar[k-1] = pairVar[k];
            symRel[k-1] = symRel[k];
        }
        pairVar[i-1] = tmp;
        symRel[i-1] = tmp2;
        //// pairVar の中身を変更
        for ( k=i-1; k>=j; k-- )
            if ( (j+1 <= pairVar[k]) && (pairVar[k] <= i-1) ) pairVar[k]--;
        pairVar[i] = i-1;

//      for ( k=n; k>1; k--) printf("%3d", pairVar[k]);
//      printf("\n");
//      for ( k=n; k>1; k--) printf("%3d", level2index(k));
//      printf("\n");
    }
}
