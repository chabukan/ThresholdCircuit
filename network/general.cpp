#include "general.h"

void cubeOrderSub(Sop cSet, int* order) {
    int n = order[0];	//// order[0] is nPI()

    int max=0, max_i=0;
    Sop relateSop;

    int top = cSet.topvar();
    uint* literals = (uint*)calloc((top<<1)+1, sizeof(int));
    cSet.getRep().litCount(literals);

    for ( int i=top; i>0; i-- ) {// タイブレイクのときは，iの大きい方が上位
        if ( order[i] ) continue;
        int appear1 = literals[ (i<<1) ];
        int appear0 = literals[ (i<<1) -1 ];
        int appear = appear0 + appear1;
        // 	fprintf(stderr, "[%d]%d ", i, appear);
        if ( appear > max ) { 
            max = appear;
            max_i = i;
        }
    }
    free(literals);

    //     fprintf(stderr, " -> [%d]%d\n", max_i, max);
    if ( max_i == 0 ) return;
    relateSop = cSet.factor0(max_i) + cSet.factor1(max_i);

    assert( order[max_i] == 0 );
    order[max_i] = n - order[n+1];	//// from top
    order[n+1]++;			//// order[n+1]: n_fix

    cubeOrderSub(relateSop, order);
    cubeOrderSub(cSet, order);
}

void cubeOrder(const Sop& cSet, vector<int>& lev2idx, int n)
{	// lev2idx: idxは1から始まる
    int*	order = (int *)calloc(n+2, sizeof(int));
    order[0] = n;
    //// order[0] は nPI()
    //// order[n+1] は # of fixed variable

    cubeOrderSub(cSet, order);

    int i;
    if ( order[n+1] != n ) {	//// order[n+1] is n_fix
        for ( i=1; i<=n; i++ ) if ( order[i] == 0 ) {
                order[i] = n-order[n+1];	//// from top
                order[n+1]++;
            }
    }

    lev2idx.resize(n+1);
    for ( i=1; i<=n; i++ ) lev2idx[ order[i] ] = i;
    lev2idx[0] = 0;
    free(order);
}
