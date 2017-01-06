#include "bddop.h"
#include "sop.h"

const Bdd Bdd::null = Bdd(BDDNULL);
const Bdd Bdd::zero = Bdd(BDDF);
const Bdd Bdd::one =  Bdd(BDDT);

const Bdd Bdd::empty = Bdd(ZBDDE);
const Bdd Bdd::base =  Bdd(ZBDDB);

const Sop Sop::null = Sop( Bdd::null );
const Sop Sop::zero = Sop( Bdd::empty );
const Sop Sop::one = Sop( Bdd::base );

