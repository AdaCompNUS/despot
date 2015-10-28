# -----------------------
# Compiler/linker options
# -----------------------

CXX = g++
LDFLAGS = -O3 -Wall -Wno-sign-compare $(GPROF)

# -----------
# Directories
# -----------

DESPOTHOME = ../../..
SRCDIR = src
INCDIR = $(DESPOTHOME)/include
LIBDIR = $(DESPOTHOME)/build
INCLUDE = -I $(INCDIR) -I $(SRCDIR)

# -----
# Files
# -----

RS_SOURCES = $(shell find -L $(SRCDIR)/base $(SRCDIR)/rock_sample -name '*.cpp')
RS_BIN = rock_sample

FVRS_SOURCES = $(shell find -L $(SRCDIR)/base $(SRCDIR)/fvrs -name '*.cpp')
FVRS_BIN = fvrs

# -------
# Targets
# -------

.PHONY: all clean 

all: RS_BIN FVRS_BIN

RS_BIN:
	@cd $(DESPOTHOME) && make
	$(CXX) $(LDFLAGS) $(RS_SOURCES) $(INCLUDE) $(LIBDIR)/*.o -o $(RS_BIN)

FVRS_BIN:
	@cd $(DESPOTHOME) && make
	$(CXX) $(LDFLAGS) $(FVRS_SOURCES) $(INCLUDE) $(LIBDIR)/*.o -o $(FVRS_BIN)


RS_BIN_LIGHT:
	@cd $(DESPOTHOME) && make
	$(CXX) $(LDFLAGS) $(RS_SOURCES) $(INCLUDE) -L $(LIBDIR) -ldespot -o $(RS_BIN)

FVRS_BIN_LIGHT:
	@cd $(DESPOTHOME) && make
	$(CXX) $(LDFLAGS) $(FVRS_SOURCES) $(INCLUDE) -L $(LIBDIR) -ldespot -o $(FVRS_BIN)

clean:
	rm -f $(RS_BIN) $(FVRS_BIN)
