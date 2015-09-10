# -----------------------
# Compiler/linker options
# -----------------------

CXX = g++
LDFLAGS = -O3 -Wall -Wno-sign-compare $(GPROF)

# -----------
# Directories
# -----------

DESPOTHOME = ../..
SRCDIR = src
INCDIR = $(DESPOTHOME)/include
LIBDIR = $(DESPOTHOME)/build
INCLUDE = -I $(INCDIR) -I $(SRCDIR)

# -----
# Files
# -----

TAG_SOURCES = $(shell find -L $(SRCDIR)/ -name '*.cpp')
TAG_BIN = tag 

# -------
# Targets
# -------

.PHONY: all clean 

all: TAG_BIN

TAG_BIN:
	@cd $(DESPOTHOME) && make
	$(CXX) $(LDFLAGS) $(LDFLAGS) $(TAG_SOURCES) $(INCLUDE) $(LIBDIR)/*.o -o $(TAG_BIN)

clean:
	rm -f $(TAG_BIN) $(LASER_TAG_BIN)
