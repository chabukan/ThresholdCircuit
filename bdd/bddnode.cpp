//////////////////////////////////////////////////////////////////
////////        BDD node management                       ////////
//////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bddnode.h"

//////////////////////////////////////////////////////////////////

static void	recfree(bddNodeT* ip);
static void	moreNode();

//////////////////////////////////////////////////////////////////

extern const int	BDDNULL = 0;
extern const int	BDDF = 1;
extern const int	BDDT = ~BDDF;
extern const int	ZBDDE = BDDF; // empty  { }
extern const int	ZBDDB = BDDT; // base {0..0}

extern const int	FIRSTNODE = 2;

//	0	1	2	3	...
//   BDDNULL   BDDF   NODE1   NODE2	...

// node[0] = { edge0=0; edge1=0; lev=0; mark=0; share=0; next=1; }
// node[1] = { edge0=1; edge1=1; lev=0; mark=0; share=0; next=2; }
// Neither node[0].inc() nor node[0].dec() are allowed.
// Neither node[1].inc() nor node[1].dec() are allowed.

extern const int        MAXLEVLOG = 14;
extern const int        MAXLEV = (1<<MAXLEVLOG) -1; //// 0011..1 == 16384-1

bddNodeT*	node;
int	ncount;
int	nsize = 0;

static int	ntsize;
static int	avail;
static int*	ntable;
static int	ntsearch = 0;
static int	maxnode;
static void	(*moreNodeEfunc)();
static void	(*gCollectEfunc)();

static int getnode_core(int f0, int f1, int lev);

//////////////////////////////////////////////////////////////////

#define nthash(f0, f1, lev, size) \
( ( (f0) + ((f1)<<2) + ((lev)<<4) + ((f1)<<6) + ((f0)<<8) ) & ((size)-1) )

void alloc_bdd_node(int initsize_power, int maxsize_power)
{
    nsize = 1<<initsize_power;
    maxnode = 1<<maxsize_power;
//    printf("sizeof(bddNodeT) = %d\n", sizeof(bddNodeT));
    node = new bddNodeT[nsize];
    assert( node );

    register int i;
    register bddNodeT* ip;
    for ( i = nsize; --i>=0; ) {
	ip = node +i;
	ip->lev = 0;
	ip->mark = 0;
	ip->share = 0;
	ip->next = i+1;
    }

    node[0].edge0 = node[0].edge1 = BDDNULL;	//// node[0] == BDDNULL
    node[1].edge0 = node[1].edge1 = BDDF;	//// node[1] == BDDF
    node[nsize-1].next = BDDNULL;

    avail = FIRSTNODE;
    ntsize = nsize;

    ntable = (int *) calloc(ntsize, sizeof(int)); // all BDDNULL
    assert( ntable );
    ncount = 0;
}

int getnode(int f0, int f1, int lev)
{
    //// return f0 if f0 == f1
    if ( f0 == f1 ) {
	int f1_ = rmN(f1);
	if ( isNode(f1_) ) node[f1_].dec();
	return f0;
    }

    int f0s = rmN(f0);
    int f1s = isN(f0) ? ~f1 : f1;
    int f = getnode_core(f0s, f1s, lev);
    return isN(f0) ? ~f : f;
}

int getzbdd(int f0, int f1, int lev)
{
    //// return f0 if f1 == BDDF
    if ( f1 == BDDF ) {
	int f_ = rmN(f1);
	if ( isNode(f_) ) node[f_].dec();
	return f0;
    }
    int f0s = rmN(f0);
    int f = getnode_core(f0s, f1, lev);
    return isN(f0) ? ~f : f;
}

int getnode_core(int f0, int f1, int lev)
{
    register int f_;

    //// return BDDNULL if f0 == BDDNULL or f1 == BDDNULL
    if ( f0 == BDDNULL ) {
	f_ = rmN(f1);
	if ( isNode(f_) ) node[f_].dec();
	return f0;
    }
    else if ( f1 == BDDNULL ) {
	f_ = rmN(f0);
	if ( isNode(f_) ) node[f_].dec();
	return f1;
    }

    assert( lev > 0 && lev <= (1<<15)-1 );
    register int f0s, f1s, key, f;
    register bddNodeT* node_f;
    f0s = rmN(f0);
    f1s = isN(f0) ? ~f1 : f1;
    key = nthash(f0s, f1s, lev, ntsize);
    f = ntable[key];
    while (f != BDDNULL) {
	node_f = node+f;
	if ( node_f->edge0 == f0s && node_f->edge1 == f1s
	    && node_f->lev == lev ) {
	    if ( isNode(f0s) ) node[f0s].dec();
	    f_ = rmN(f1s);
	    if ( isNode(f_) ) node[f_].dec();
	    break;
	}
	f = node_f->next;
	ntsearch++;
    }
    if ( f == BDDNULL ) {
	if ( avail == BDDNULL ) {
	    if ( (ncount<<1) < maxnode ) moreNode();
	    if ( avail == BDDNULL ) gCollect();
 	    if ( avail == BDDNULL ) abort_bdd((char*)"in getnode(): node overflow");
// 	    if ( avail == BDDNULL ) {
// 		fprintf(stderr, "in getnode(): node overflow\n");
// 		return BDDNULL;
// 	    }
	    key = nthash(f0s, f1s, lev, ntsize);
	}
	f = avail;
	node_f = node+f;
	avail = node_f->next;
	ncount++;
	node_f->edge0 = f0s;
	node_f->edge1 = f1s;
	node_f->lev = lev;
	node_f->next = ntable[key];
	ntable[key] = f;
    }
    node[f].inc();
    return isN(f0) ? ~f : f;
}

void gCollect()
{
    register int i;

    fprintf(stderr, "gCollect(): ncount = %d -> ", ncount);
    for ( i = nsize; --i>0; ) {
	bddNodeT* ip = node +i;
	if ( ip->lev > 0 ) if ( ip->share == 0 ) {
	    recfree(ip);
	}
    }
    fprintf(stderr, "%d\n", ncount);
    (*gCollectEfunc)();
}

void printUnfreedNode()
{
    for ( int i = nsize; --i>0; ) if ( node[i].share > 0 ) {
	fprintf(stderr, "printUnfreedNode: [%d](lev:%d, e0:%d, e1:%d, s:%d)\n",
		i, node[i].lev, node[i].edge0, node[i].edge1, node[i].share);
    }
}

void setEfunc(void (*f)(), void (*g)())
{
    moreNodeEfunc = f;
    gCollectEfunc = g;
}

void clearMark(int f)
{
    int f_ = rmN(f);
    if ( node[f_].mark == 0 ) return;
    node[f_].mark = 0;
    if ( isNode(f_) ) {
	clearMark( node[f_].edge0 );
	clearMark( node[f_].edge1 );
    }
}

//// check all the path: take a long time
int checkMark(int f)
{
    int f_ = rmN(f);
    assert( node[f_].mark == 0 );
    if ( node[f_].mark ) return 1;
    if ( isNode(f_) == 0 ) return 0;
    return checkMark( node[f_].edge0 ) | checkMark( node[f_].edge1 );
}

void checkMark()
{
    for (int i=nsize-1; i>0; i--) assert( node[i].mark == 0 );
}

void abort_bdd(char* errmsg)
{
    fprintf(stderr, "Error[BDD]!! %s\n", errmsg);
    abort();
}

void abort_bdd(char* errmsg, int f)
{
    fprintf(stderr, "Error[BDD]!! %s (%d)\n", errmsg, f);
    abort();
}

void bddNodeT::incError() {
    share--;
    gCollect();
    if ( ++share == 0 ) abort_bdd((char*)"in inc(): share counter overflow");
}

void bddNodeT::decError() {
    abort_bdd((char*)"in dec(): share counter underflow");
}

void bddNodeT::decError(int f) {
    abort_bdd((char*)"in dec(): share counter underflow", f);
}

static void recfree(register bddNodeT* fp)
{
    int f0 = fp->edge0;
    int f1 = fp->edge1;
    int key = nthash(f0, f1, fp->lev, ntsize);

    register int* gp = &(ntable[key]);
    register int h = *gp;
    register int f = fp -node;
    while ( h != f ) {
	gp = &(node[h].next);
	h = *gp;
	ntsearch++;
    }
    // h == f, node+h == fp
    *gp = fp->next;
    fp->next = avail;
    avail = f;
    fp->lev = 0;
    fp->mark = 0;
    ncount--;

    // fp と f を別の意味で使っている (効率のため)
    if ( isNode(f = rmN(f0)) ) {
	fp = node +f;
	fp->dec(f);
	if ( fp->share == 0 ) recfree( fp );
    }
    if ( isNode(f = rmN(f1)) ) {
	fp = node +f;
	fp->dec(f);
	if ( fp->share == 0 ) recfree( fp );
    }
}

static void moreNode()
{
//     printf("moreNode(): nsize = %d -> %d\n", nsize, nsize<<1);
    //////// enlarge node ////////
    bddNodeT* newnode = new bddNodeT[nsize<<1];
    if ( newnode == 0 ) return;
    memcpy(newnode, node, sizeof(bddNodeT)*nsize);
    delete [] node;
    node = newnode;
    //////// initializing new nodes ////////
    register int i;
    for (i=nsize; i<=(nsize<<1)-1; i++) {
        node[i].lev = 0;
	node[i].mark = 0;
        node[i].share = 0;
        node[i].next = i+1;
    }
    node[(nsize<<1)-1].next = avail;
    avail = nsize;
    nsize<<=1;

    //////// enlarge ntable ////////
    int* newntable = (int*)malloc( (ntsize<<1)*sizeof(int) );
//     int* newntable = new int[ntsize<<1];
    if ( newntable == 0 ) return;
    memcpy(newntable, ntable, sizeof(int)*ntsize);
    free(ntable);
    ntable = newntable;

    //////// rehashing node table ////////
    register int key, f, *gp;
    for ( i=ntsize-1; i>=0; i-- ) {
        ntable[i+ntsize] = BDDNULL;
        gp = &(ntable[i]);
        f = *gp;
        while ( f != BDDNULL ) {
            key = nthash( node[f].edge0, node[f].edge1, 
			 node[f].lev, ntsize<<1 );
            if ( key == i+ntsize ) {
                *gp = node[f].next;
                node[f].next = ntable[i+ntsize];
                ntable[i+ntsize] = f;
            }
            else if ( key == i )
                gp = &(node[f].next);
            else
                assert(0);
            f = *gp;
        }
    }
    ntsize<<=1;

    (*moreNodeEfunc)();
}
    
