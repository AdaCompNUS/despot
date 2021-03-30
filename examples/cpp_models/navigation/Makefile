# -----------------------
# Compiler/linker options
# -----------------------

GPROF = # avoid externally defined GPROF
#GPROF = -pg # uncomment to enable gprof

CXX = g++
LDFLAGS = -O3 -Wall -Wno-sign-compare $(GPROF)

# -----------
# Directories
# -----------

DESPOTHOME = ../../..
SRCDIR = src
INCDIR = $(DESPOTHOME)/include
LIBDIR = $(DESPOTHOME)/build

# -----
# Files
# -----

SOURCES = $(shell find -L $(SRCDIR) -name '*.cpp')
BIN = navigation 

# -------
# Targets
# -------

.PHONY: all clean 

all:
	@cd $(DESPOTHOME) && make
	$(CXX) $(LDFLAGS) $(SOURCES) -I $(INCDIR) $(LIBDIR)/*.o -o $(BIN)

light:
	@cd $(DESPOTHOME) && make
	$(CXX) $(LDFLAGS) $(SOURCES) -I $(INCDIR) -L $(LIBDIR) -ldespot -o $(BIN)

clean:
	rm -f $(BIN)
