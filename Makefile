# -----------------------
# Compiler/linker options
# -----------------------

CXX = g++
CXXFLAGS = -O3 -c -Wall -Wno-sign-compare
LDFLAGS = -O3 -Wno-sign-compare

# -----------
# Directories
# -----------

SRCDIR = src
BINDIR = bin
OBJDIR = lib
DEPDIR = .deps
PROGLIB = $(OBJDIR)/despot.a

POMDPX_PROG = $(BINDIR)/pomdpx_solver

# -----
# Files
# -----

VPATH = $(shell find -L $(SRCDIR) -type d \( ! -name '.*' \) -not -path "src/problems/*" )
SOURCES = $(shell find -L $(SRCDIR) -name '*.cpp' -not -path "src/problems/*" -not -name 'pomdpx_main.cpp' )
OBJS = $(addprefix $(OBJDIR)/, $(patsubst %.cpp, %.o, $(notdir $(SOURCES))))
DEPS = $(addprefix $(DEPDIR)/, $(patsubst %.cpp, %.d, $(notdir $(SOURCES))))
INCL = -I $(SRCDIR)
LIBS = $(PROGLIB)

BINS = $(POMDPX_PROG)

# -------
# Targets
# -------

.PHONY: all clean

all: DIR_TGTS $(DEPS) $(LIBS) $(BINS) 

DIR_TGTS:
	mkdir -p $(OBJDIR) $(DEPDIR) $(BINDIR)

$(PROGLIB): $(OBJS)
	ar crf $(PROGLIB) $(OBJS)

$(POMDPX_PROG): $(OBJS) pomdpx_main.o problem_solver.o simulator.o
	$(CXX) $(OBJS) $(OBJDIR)/pomdpx_main.o $(OBJDIR)/problem_solver.o $(OBJDIR)/simulator.o $(LDFLAGS) $(INCL) -o $(POMDPX_PROG)

$(DEPDIR)/%.d: %.cpp
	@mkdir -p $(DEPDIR); \
	$(CXX) -MM $(CXXFLAGS) $(INCL) $< > $@; \
	sed -ie 's;\(.*\)\.o:;$(OBJDIR)/\1.o $(DEPDIR)/\1.d:;g' $@

-include $(DEPS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCL) $< -o $@ 

pomdpx_main.o: pomdpx_main.cpp
	$(CXX) $(CXXFLAGS) $(INCL) src/pomdpx/pomdpx_main.cpp -o $(OBJDIR)/pomdpx_main.o 

problem_solver.o: problem_solver.cpp
	$(CXX) $(CXXFLAGS) $(INCL) src/problems/problem_solver.cpp -o $(OBJDIR)/problem_solver.o 

simulator.o: simulator.cpp
	$(CXX) $(CXXFLAGS) $(INCL) src/problems/simulator.cpp -o $(OBJDIR)/simulator.o 

clean:
	rm -rf $(OBJDIR) $(BINDIR) $(DEPDIR)





