
CXX=g++
LD=ld
CFLAGS=-std=c++17 -I../fmt/include
LFLAGS=-L../fmt/build
LIBS=-lgtest -lgtest_main -lpthread -lfmt

all: fmtstertest

fmtstertest.o: fmtstertest.cpp fmtster.h Makefile
	$(CXX) $(CFLAGS) -c $< -o $@

fmtstertest: fmtstertest.o
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS) $(LIBS)
	strip $@

insoptest.o: insoptest.cpp Makefile
	$(CXX) $(CFLAGS) -c $< -o $@

insoptest: insoptest.o
	$(CXX) $(CFLAGS) $^ -o $@ $(LFLAGS) $(LIBS)
	strip $@
