## object files ##

## -m32 or -m64
BTYPE = -m64

CFLAGS = -std=c++11 -g -Wall -DGCC
# CFLAGS = -O3 -std=c++11 -g -Wall -DGCC

CXX = g++

SRCS = bddnode.cpp bddc.cpp opsingle.cpp opbinary.cpp bddop.cpp \
       bddtrav.cpp bddecomp.cpp isf.cpp isf-ger.cpp factor.cpp \
       ordbdd.cpp sop.cpp const.cpp cube.cpp


OBJS = ${SRCS:.cpp=.o}

## header files ##

TOP = ..

INC-BDD  = -I${TOP}/bdd
INC-UTIL = -I${TOP}/util
#INC-GER  = -I${TOP}/ger-base
# todo
INC-UTIL2 = -I${TOP}/network

MY-INC = ${INCGCC} ${INC-BDD} ${INC-UTIL} ${INC-GER} ${INC-UTIL2}

INC-G++ = -I/usr/gnu/include/g++

## libraries ##

ARCS = libbdd.a
SO = libbdd.so


.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) $(BTYPE) $(CFLAGS) -c $< ${MY-INC}

all: ${ARCS}

${ARCS}: ${OBJS}
	rm -f $@
	ar cr $@ ${OBJS}

${SO}: ${OBJS}
	$(CXX) $(BTYPE) -G -o $@ ${OBJS} ${MY-INC}

depend:
	makedepend ${SRCS} -DGCC ${MY-INC} ${INC-G++}

clean:
	rm -f *.bak *.o test *.*~ \#*\# *.a *.so

test: test.cpp ${ARCS} ${TOP}/util/libutil.a ${TOP}/ger-base/libger-base.a
	g++ -I../util -I. $^ -o $@

${TOP}/util/libutil.a:
	cd ${TOP}/util; ${MAKE} "CFLAGS=$(CFLAGS)"
