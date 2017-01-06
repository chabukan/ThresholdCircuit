#include "factor.h"
#include "ger-base.h"
#include <iostream>
#include <cstdlib>
#include <unistd.h>

Factor::Factor(const Factor& ft) : type(ft.type), cset(ft.cset)
{
    if ( type == INT ) copy_son(ft);
}

void Factor::clear()
{
    if ( type == INT ) {
	delete fac_q;
	delete fac_d;
	delete fac_r;
    }
    cset = Sop::null;
    type = EMPTY;
}

void Factor::copy_son(const Factor& ft)
{
    fac_q = new Factor(*ft.fac_q);
    fac_d = new Factor(*ft.fac_d);
    fac_r = new Factor(*ft.fac_r);
}

Factor&	Factor::operator = (const Factor& ft)
{
    clear();

    type = ft.type;
    cset = ft.cset;

    if ( type == INT ) copy_son(ft);

    return *this;
}

void Factor::makeCubeFree()
{
    if ( type == EMPTY ) return;
    else if ( type == LEAF ) {
        if ( cset.dupLit() != Sop::null ) sop2factor();
    }
    else { // INT
        fac_q->makeCubeFree();
        fac_d->makeCubeFree();
        fac_r->makeCubeFree();
    }
}

void Factor::sop2factor()
{
    const int LOG = 0;

    assert( type == LEAF );

    Sop d = cset.getKernel();
    if ( cset == d ) return;	// cube free

    if ( LOG ) {
	fprintf(stderr, "cset:\n"); CubeSet::printSop(cset);
	fprintf(stderr, "d:\n"); CubeSet::printSop(d);
    }
    Sop q = cset / d;
    if ( LOG ) {
	fprintf(stderr, "q:\n"); CubeSet::printSop(q);
    }

    if ( q.nCube() == 1 ) {	// literal factor
	if ( LOG ) {
	    fprintf(stderr, "literal_factor: ");
	    CubeSet::printSop(q);
	}
	literal_factor(q);
    }
    else {
	Sop cq = q.commonCube();
	q = q / cq;
	d = cset / q;
	Sop cd = d.commonCube();

	if ( cd != Sop::one ) {	// literal factor
	    if ( LOG ) {
		fprintf(stderr, "literal_factor: ");
		CubeSet::printSop(q);
	    }
	    literal_factor(cd);
	}
	else {			// kernel factor
	    Sop r = cset - d * q;
	    cset = Sop::null;
	    type = INT;

	    if ( LOG ) {
		fprintf(stderr, "kernel_factor:\n");
		CubeSet::printSop(q);
	    }
	    fac_q = new Factor(q);
	    fac_d = new Factor(d);
	    fac_r = new Factor(r);
	    makeCubeFree();
	}
    }
}

void Factor::literal_factor(const Sop& cube)
{
    Sop lit = cset.most_common_literal(cube);
    Sop d1 = cset / lit;
    Sop r = cset - d1 * lit;

    Sop c = d1.commonCube();
    Sop d = d1 / c;
    Sop q = lit * c;

    cset = Sop::null;
    type = INT;
    fac_q = new Factor(q);
    fac_d = new Factor(d);
    fac_r = new Factor(r);
    makeCubeFree();
}

void Factor::print()
{
    if ( type == LEAF ) {
	if ( cset != Sop::zero ) {
	    Bdd f = cset.getFunc();
	    if ( f == Bdd::one ) abort();

	    sop_print(stderr, cset, 0 );
	}
    }
    else {
	std::cerr << "Q( ";
	fac_q->print();
	std::cerr << " ) D( ";
	fac_d->print();
	std::cerr << " )";
	std::cerr << " + R ";
	fac_r->print();
    }
}

int Factor::nLit()
{
    if ( type == EMPTY ) return 0;
    else if ( type == LEAF ) {
	return cset.nLit();
    }
    else { // INT
	int sum = 0;
	sum += fac_q->nLit();
	sum += fac_d->nLit();
	sum += fac_r->nLit();
	return sum;
    }
}

int Factor::n_appear(int level)
{
    if ( type == EMPTY ) return 0;
    else if ( type == LEAF ) {
	int nlit = cset.factor0(level).nCube();
	int plit = cset.factor1(level).nCube();
	return nlit + plit;
    }
    else { // INT
	int sum = 0;
	sum += fac_q->n_appear(level);
	sum += fac_d->n_appear(level);
	sum += fac_r->n_appear(level);
	return sum;
    }
}
