#ifndef _bddnode_h
#define _bddnode_h

extern const int FIRSTNODE;

#define isN(f) ((f) < 0)
#define rmN(f) (((f) < 0) ? ~(f) : (f))
#define isNode(f) ((f) >= FIRSTNODE)

typedef unsigned int	u_int;

struct bddNodeT {
    int		edge0;
    int		edge1;
    short	lev;
    short	mark;
    u_int	share;
    int		next;

    void	inc() { if ( ++share == 0 ) incError(); }
    void	dec() { if ( share-- == 0 ) decError(); }
    void	dec(int f) { if ( share-- == 0 ) decError(f); }
    void	incError();
    void	decError();
    void	decError(int f);
};

extern void	gCollect();

extern void	alloc_bdd_node(int size, int maxsize);
extern int	getnode(int f0, int f1, int lev);
extern int	getzbdd(int f0, int f1, int lev);
extern void	setEfunc(void (*f)(), void (*g)());
extern void	abort_bdd(char* errmsg);
extern void	abort_bdd(char* errmsg, int f);
extern void	clearMark(int f);
extern int	checkMark(int f);
extern void	checkMark();
extern void     printUnfreedNode();

extern bddNodeT*	node;
extern int	nsize;
extern int	ncount;

inline int rstrtopup(int f, int lev) // BDDF: lev, BDDT: ~lev
{
    int f_ = rmN(f);
    int flev = node[f_].lev;
    int lev_ = rmN(lev);
    if ( flev == lev_ ) {
	int h = isN(lev) ? node[f_].edge1 : node[f_].edge0;
	return ( isN(f) ? (~h) : h );
    }
    else return f;	//// flev < lev_
}

inline int rstrtop(int f, int val) // val: BDDF or BDDT ‚ðŠú‘Ò
{
    int neg = isN(f);
    if ( neg ) f = ~f;	//// f became positive
    int h = isN(val) ? node[f].edge1 : node[f].edge0;
    return ( neg ? (~h) : h );
}

inline int subsettop(int f, int val) // val: BDDF or BDDT ‚ðŠú‘Ò (ZBDD)
{
    int f_ = rmN(f);
    int h_ = isN(val) ? node[f_].edge1 : node[f_].edge0; // ~val: subset1
    return (isN(val) || !isN(f)) ? h_ : ~h_;
    // subset1‚Ìê‡‚Í”Û’èƒGƒbƒW‚ð“`‚¦‚È‚¢
}

#endif
