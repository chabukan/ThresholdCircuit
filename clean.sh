#!/bin/sh

(
if [ -e "GPATH" ]; then
	rm -v GPATH
fi
if [ -e "GRTAGS" ]; then
	rm -v GRTAGS
fi
if [ -e "GTAGS" ]; then
	rm -v GTAGS
fi
cd main/
make clean
cd ../bdd/
make clean
cd ../network/
make clean
cd ../util/
make clean
cd ..
)
