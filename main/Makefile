TOP = ..
CUDD-TOP = ${TOP}/cudd-2.5.0

INC-BDD  = -I${TOP}/bdd
INC-UTIL = -I${TOP}/util
INC-NET = -I${TOP}/network
INC-CUDD = -I$(CUDD-TOP)/include
MY-INC = ${INCGCC} ${INC-BDD} ${INC-UTIL} ${INC-NET} ${INC-CUDD}

#LIB-CUDD =  -L$(CUDD-TOP)/obj -L$(CUDD-TOP)/cudd -L$(CUDD-TOP)/mtr -L$(CUDD-TOP)/st -L$(CUDD-TOP)/util -L$(CUDD-TOP)/epd
#LIBS = -lobj -lcudd -lmtr -lst -lutil -lepd

#MYLIBS = \
#${TOP}/network/libnetwork.a \
#${TOP}/bdd/libbdd.a \
#${TOP}/util/libutil.a \
#${CUDD-TOP}/cudd/libcudd.a \
#${CUDD-TOP}/mtr/libmtr.a \
#${CUDD-TOP}/st/libst.a \
#${CUDD-TOP}/util/libutil.a \
#${CUDD-TOP}/epd/libepd.a \
#${CUDD-TOP}/obj/libobj.a \
# ${CUDD-TOP}/cudd/libcudd.a \
# ${CUDD-TOP}/st/libst.a \
# ${CUDD-TOP}/obj/libobj.a \
# ${CUDD-TOP}/util/libutil.a \
# ${CUDD-TOP}/mtr/libmtr.a \
# ${CUDD-TOP}/epd/libepd.a \

MYLIBS = \
${TOP}/network/libnetwork.a \
${TOP}/bdd/libbdd.a \
${TOP}/util/libutil.a \
${CUDD-TOP}/obj/libobj.a \
${CUDD-TOP}/cudd/libcudd.a \
${CUDD-TOP}/mtr/libmtr.a \
${CUDD-TOP}/st/libst.a \
${CUDD-TOP}/epd/libepd.a \
${CUDD-TOP}/util/libutil.a

###########################

## -m32 or -m64
BTYPE = -m64

CXX = g++

TARGET = ger-program

CXXFLAGS = -std=c++11 -Wall -O2 -pipe
# CXXFLAGS = -std=c++11 -Wall -O2 -pipe -g 

SRCS = main.cpp
OBJS := $(SRCS:.cpp=.o) #SRCSの各ファイルのサフィックスの.cppを.oに変換

.cpp.o:
	$(CXX) $(BTYPE) $(CXXFLAGS) -c $< ${MY-INC}

$(TARGET): $(OBJS)
	$(CXX) $(BTYPE) -o $@ $(OBJS) ${MYLIBS}  -lstdc++ -lm
#${LIB-CUDD} ${LIBS}

clean:
	rm -f $(TARGET) $(OBJS) *.*~ \#*\#
