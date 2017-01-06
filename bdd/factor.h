#ifndef _factor_h
#define _factor_h

#include "sop.h"

class Factor {
public:
    Factor() : type(EMPTY) {}
    Factor(const Sop& f) : type(LEAF), cset(f) {}
    Factor(const Factor& ft);

    ~Factor() { clear(); }

    Factor&	operator = (const Factor& ft);

    void	clear();
    int		isEmpty() { return (type == EMPTY); }

    void	makeCubeFree();
    void	print();
    int		nLit();
    int		n_appear(int level);

private:
    void	copy_son(const Factor& ft);

    void	sop2factor();
    void	literal_factor(const Sop& cube);

    enum {EMPTY, LEAF, INT}	type;
    Sop				cset;
    Factor*			fac_q;
    Factor*			fac_d;
    Factor*			fac_r;
};

#endif
