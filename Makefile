# -----------------------
# Compiler/linker options
# -----------------------

CXX = g++
CXXFLAGS = -O3 -c -Wall -Wno-sign-compare
LDFLAGS = -O3 -Wno-sign-compare

# -----------
# Directories
# -----------

DESPOT_SRCDIR = src/despot_library
POMDPX_SRCDIR = src/pomdpx_parser
BINDIR = bin
OBJDIR = lib
DEPDIR = .deps
PROGLIB = $(OBJDIR)/despot.a
POMDPX_PROG = $(BINDIR)/despot_pomdpx

# -----
# Files
# -----

VPATH = $(shell find -L $(DESPOT_SRCDIR) -type d \( ! -name '.*' \) ) $(shell find -L $(POMDPX_SRCDIR) -type d \( ! -name '.*' \) )

# parameters used for despot
DESPOT_SOURCES = $(shell find -L $(DESPOT_SRCDIR) -name '*.cpp')
DESPOT_OBJS = $(addprefix $(OBJDIR)/, $(patsubst %.cpp, %.o, $(notdir $(DESPOT_SOURCES))))

# parameters used for pomdpx
POMDPX_SOURCES = $(shell find -L $(POMDPX_SRCDIR) -name '*.cpp')
POMDPX_OBJS = $(addprefix $(OBJDIR)/, $(patsubst %.cpp, %.o, $(notdir $(POMDPX_SOURCES))))

# common use

DEPS = $(addprefix $(DEPDIR)/, $(patsubst %.cpp, %.d, $(notdir $(DESPOT_SOURCES)))) $(addprefix $(DEPDIR)/, $(patsubst %.cpp, %.d, $(notdir $(POMDPX_SOURCES))))
INCL = -I $(DESPOT_SRCDIR) -I $(POMDPX_SRCDIR)

LIBS = $(PROGLIB)
BINS = $(POMDPX_PROG)

# -------
# Targets
# -------

.PHONY: all clean

all: DIR_TGTS $(DEPS) $(LIBS) $(BINS) 

DIR_TGTS:
	mkdir -p $(OBJDIR) $(DEPDIR) $(BINDIR)

$(PROGLIB): $(DESPOT_OBJS)
	ar cr $(PROGLIB) $(DESPOT_OBJS)

$(POMDPX_PROG): $(DESPOT_OBJS) $(POMDPX_OBJS)
	$(CXX) $(DESPOT_OBJS) $(POMDPX_OBJS) $(LDFLAGS) $(INCL) -o $(POMDPX_PROG)

$(DEPDIR)/%.d: %.cpp
	@mkdir -p $(DEPDIR); \
	$(CXX) -MM $(CXXFLAGS) $(INCL) $< > $@; \
	sed -ie 's;\(.*\)\.o:;$(OBJDIR)/\1.o $(DEPDIR)/\1.d:;g' $@

-include $(DEPS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCL) $< -o $@ 



clean:
	rm -rf $(OBJDIR) $(BINDIR) $(DEPDIR)





