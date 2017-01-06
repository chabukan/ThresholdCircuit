#ifndef _twobdd_h
#define _twobdd_h

#include "bddop.h"

#include <utility.h>

using namespace std;


class TwoBdd {
private:
    Bdd bit1;
    Bdd bit2;
public:
    TwoBdd() : bit1(), bit2() {}
    TwoBdd(const TwoBdd& f) : bit1(f.Bit1()), bit2(f.Bit2()) {}
    TwoBdd(const Bdd& b1, const Bdd& b2) : bit1(b1), bit2(b2) {}

    Bdd Bit1() const { return bit1; }
    Bdd Bit2() const { return bit2; }
    void change_bit1(const Bdd& f) { bit1 = f; }
    void change_bit2(const Bdd& f) { bit2 = f; }

    TwoBdd&	operator = (const TwoBdd& a) {
	bit1 = a.Bit1();
	bit2 = a.Bit2();
	return *this;
    }

    friend int operator == (const TwoBdd& lhs, const TwoBdd& rhs) {
	return (lhs.Bit1() == rhs.Bit1()) && (lhs.Bit2() == rhs.Bit2());
    }
    friend int operator != (const TwoBdd& lhs, const TwoBdd& rhs) {
	return !(lhs == rhs);
    }

    int top() const {
	int top1 = bit1.top();
	int top2 = bit2.top();
	return (top1 >= top2) ? top1 : top2;
    }
    Bdd varSet() const {
	Bdd vs1 = bit1.varSet();
	Bdd vs2 = bit2.varSet();
	return vs1 + vs2;
    }

    TwoBdd	levshift(int lev, int degree) const {
	TwoBdd result;
	result.bit1 = Bit1().levshift(lev, degree);
	result.bit2 = Bit2().levshift(lev, degree);
	return result;
    }
    TwoBdd	compose(int level, const Bdd& g) const {
	TwoBdd result;
	result.bit1 = Bit1().compose(level, g);
	result.bit2 = Bit2().compose(level, g);
	return result;
    }
    TwoBdd      changeOrder(const vector<int>& permutation) const {
	TwoBdd result;
	result.bit1 = Bit1().changeOrder(permutation);
	result.bit2 = Bit2().changeOrder(permutation);
	return result;
    }
};

#endif
