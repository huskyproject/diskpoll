# include Husky-Makefile-Config
include ../huskymak.cfg

ifeq ($(DEBUG), 1)
  CFLAGS=$(WARNFLAGS) $(DEBCFLAGS)
  LFLAGS=$(DEBLFLAGS)
else
  CFLAGS=$(WARNFLAGS) $(OPTCFLAGS)
  LFLAGS=$(OPTLFLAGS)
endif

SRC_DIR=src/


CDEFS= -DINCS_NEED_DOT_H -DNOEXCEPTIONS -D$(OSTYPE) $(ADDCDEFS) -Isrc -DCFGDIR=\"$(CFGDIR)\"

.SUFFIXES:
.SUFFIXES: .cc .c .o

%$(OBJ): $(SRC_DIR)%.cc
	$(GPP) -c $(CFLAGS) $(CDEFS) $(SRC_DIR)$*.cc

OBJFILES = \
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

all: diskpoll$(EXE)

diskpoll: $(OBJFILES)
	$(GPP) $(LFLAGS) -o diskpoll$(EXE) $(OBJFILES)

install:
	$(INSTALL) $(IBOPT) diskpoll$(EXE) $(BINDIR)

uninstall:
	-$(RM) $(RMOPT) $(BINDIR)$(DIRSEP)diskpoll$(EXE)

clean:
	-$(RM) $(RMOPT) *$(OBJ)
	-$(RM) $(RMOPT) core
	-$(RM) $(RMOPT) *~

distclean: clean
	-$(RM) $(RMOPT) diskpoll$(EXE)

