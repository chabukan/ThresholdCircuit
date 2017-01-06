#include "bddc.h"
#include <stdio.h>

main()
{
    bddalloc(16,20);

    bddid a = bddvar(1);
    bddid b = bddvar(2);
    bddid c = bddvar(3);

    bddid f0 = bddor( a, b );
    bddid f = bddand( f0, c );

    fprintf(stderr, "size = %d\n", bddsize(f));
    bddprintbdl(f, stderr);

    bddfree(a);
    bddfree(b);
    bddfree(c);
    bddfree(f0);
    bddfree(f);

    bddgc();
}

