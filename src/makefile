# Makefile for GCC on any UNIX platform

GPP=gcc
CFLAGS=-DNOEXCEPTIONS -Wall -pedantic

.cc.o:
	$(GPP) -c $(CFLAGS) $<

OBJFILES = \
 main.o \
 cnode.o \
 cstring.o \
 coutb.o \
 cbinkout.o \
 findfile.o \
 copyfile.o \
 cinbound.o \
 csystem.o \
 words.o \
 csplout.o \
 prepcfg.o \
 log.o

default: test$(EXE)

test$(EXE): $(OBJFILES)
	$(GPP) $(CFLAGS) -o diskpoll$(EXE) $(OBJFILES) -lstdc++ -s

clean:
	-rm *.o
	-rm core
	-rm diskpoll$(EXE)
	-rm *~

