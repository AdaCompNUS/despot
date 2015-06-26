# -----------------------
# Compiler/linker options
# -----------------------

CXX = g++
CXXFLAGS = -O3 -c -Wall -Wno-sign-compare
LDFLAGS = -O3 -Wno-sign-compare

# -----------
# Directories
# -----------

SRCDIR = ../../../src
BINDIR = ../../../src/problems/bridge/bin
OBJDIR = ../../../src/problems/bridge/build
DEPDIR = ../../../src/problems/bridge/.deps
PROG = $(BINDIR)/bridge
#INCLUDE = ../../../include

# -----
# Files
# -----

VPATH = $(shell find -L $(SRCDIR)/problems -type d \( ! -name '.*' \))
SOURCES = $(shell find -L $(SRCDIR)/problems -maxdepth 1 -name '*.cpp') \
          $(shell find -L $(SRCDIR)/problems/bridge -name '*.cpp')
OBJS = $(addprefix $(OBJDIR)/, $(patsubst %.cpp, %.o, $(notdir $(SOURCES))))
DEPS = $(addprefix $(DEPDIR)/, $(patsubst %.cpp, %.d, $(notdir $(SOURCES))))
BINS = $(PROG)
#INCL = -I $(INCLUDE) -I $(SRCDIR)
INCL = -I $(SRCDIR)

# -------
# Targets
# -------

.PHONY: all clean

all: DIR_TGTS $(DEPS) $(BINS)

DIR_TGTS:
	mkdir -p $(BINDIR) $(OBJDIR) $(DEPDIR)

$(PROG): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(INCL) -o $(PROG) ../../../lib/despot.a; \
#	rm -r build

$(DEPDIR)/%.d: %.cpp
	@mkdir -p $(DEPDIR); \
	$(CXX) -MM $(CXXFLAGS) $(INCL) $< > $@; \
	sed -ie 's;\(.*\)\.o:;$(OBJDIR)/\1.o $(DEPDIR)/\1.d:;g' $@

-include $(DEPS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCL) $< -o $@ 

clean:
	rm -rf $(OBJDIR) $(BINDIR) $(DEPDIR)


