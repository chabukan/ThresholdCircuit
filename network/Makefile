## object files ##

## CUDD
CUDD-TOP = ../cudd-2.5.0

## -m32 or -m64
BTYPE = -m64

CFLAGS = -std=c++11 -g -Wall -DGCC
# CFLAGS = -std=c++11 -g -Wall -DGCC -Wno-sign-compare
# CFLAGS = -O2 -std=c++11 -g -Wall -DGCC

CXX = g++
SRCS = node.cpp network.cpp general.cpp utility.cpp cev.cpp
OBJS = ${SRCS:.cpp=.o}

## header files ##
TOP = ..

INC-NET  = -I${TOP}/network
INC-BDD  = -I${TOP}/bdd
INC-UTIL = -I${TOP}/util
INC-CUDD = -I$(CUDD-TOP)/include
MY-INC = ${INCGCC} ${INC-NET} ${INC-BDD} ${INC-GER} ${INC-UTIL} ${INC-CUDD}

INC-G++ = -I/usr/gnu/include/g++ -I/usr/gnu/sparc-sun-solaris2.6/include

# cudd
#LIB-CUDD = -L$(CUDD-TOP)/obj -L$(CUDD-TOP)/cudd -L$(CUDD-TOP)/mtr -L$(CUDD-TOP)/st -L$(CUDD-TOP)/util -L$(CUDD-TOP)/epd
#LIBS = -lobj -lcudd -lmtr -lst -lutil -lepd

## libraries ##

ARCS = libnetwork.a
SO = libnetwork.so

.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) $(BTYPE) $(CFLAGS) -c $< ${MY-INC}
#${LIB-CUDD} ${LIBS}

#all: ${SO} ${ARCS}
all: ${ARCS}

${ARCS}: ${OBJS}
	ar cr $@ ${OBJS}
	ranlib $@

${SO}: ${OBJS}
	$(CXX) $(BTYPE) -G -o $@ ${OBJS} ${MY-INC}

depend:
	makedepend ${SRCS} -DGCC ${MY-INC} ${INC-G++}

clean:
	rm -f *.bak *.o *.*~ \#*\# *.a *.so

