# Makefile for Windows 95/NT using the RSXNT GNU compiler

GPP=gcc
CFLAGS=-D__NT__ -O3 -Wall -pedantic -Zwin32 -Zsys -DNOEXCEPTIONS
EXE=.exe
OBJ=.o

%$(OBJ) : %.cc
	 $(GPP) -c $(CFLAGS) $<

OFILES = \
 main$(OBJ) \
 cnode$(OBJ) \
 cstring$(OBJ) \
 coutb$(OBJ) \
 cbinkout$(OBJ) \
 findfile$(OBJ) \
 copyfile$(OBJ) \
 cinbound$(OBJ) \
 csystem$(OBJ) \
 words$(OBJ) \
 csplout$(OBJ) \
 prepcfg$(OBJ) \
 log$(OBJ)

default: diskpoll$(EXE)

diskpoll$(EXE): $(OFILES)
	$(GPP) $(CFLAGS) -o diskpoll$(EXE) $(OFILES) -lstdcpp -s

clean:
	-cmd /c del *.o
	-cmd /c del *.obj
	-cmd /c del core
	-cmd /c del *~

distclean: clean
	-cmd /c del diskpoll$(EXE)

