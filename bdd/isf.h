#ifndef _isf_h
#define _isf_h

#include "twobdd.h"
#include "sop.h"

class IsfFD : public TwoBdd { // F: onset, D: dcset
public:
    IsfFD() : TwoBdd() {} //bddf で初期化
    IsfFD(const IsfFD& f) : TwoBdd(f) {}
    IsfFD(const TwoBdd& f) : TwoBdd(f) {}
    IsfFD(const Bdd& F, const Bdd& D) : TwoBdd(F, D) {}
    Bdd getF() const { return Bit1(); }
    Bdd getD() const { return Bit2(); }
};

class IsfLU : public TwoBdd { // Interval: [L, U]
public:
    IsfLU() : TwoBdd() {} //bddf で初期化
    IsfLU(const IsfLU& f) : TwoBdd(f) {}
    IsfLU(const TwoBdd& f) : TwoBdd(f) {}
    IsfLU(const Bdd& L, const Bdd& U) : TwoBdd(L, U) {}
    IsfLU(const Bdd& F) : TwoBdd(F, F) {}
    Bdd getL() const { return Bit1(); }
    Bdd getU() const { return Bit2(); }

    IsfLU(const IsfFD& f) {
	Bdd F = f.getF();
	Bdd D = f.getD();
	Bdd L = F & (~D);
	Bdd U = F | D;
	*this = IsfLU(L, U);
    }

#include "isf-ger.h"

    Sop makeISOPorder(const Arrayint& order, int idx);

    Sop getBetterSop(int& polarity);

    Bdd minDc() const;
    int min_support(int max_sup);

    int sameVarSet(const Bdd& f);
    void forceVarSet(const Arrayint& varset); // varsetで表される変数集合に
    void forceLitSet(const Arrayint& litset); // litsetで表されるリテラル集合に

    void newCareSet(const Bdd& careset) {
	Bdd L = getL();
	Bdd U = getU();
	L = L & careset;
	U = U | ~careset; // ~(~U & careset);
	*this = IsfLU(L, U);
    }
    void elimVar(int var);
    void elimLit0(int var);
    void elimLit1(int var);
};

#endif
