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

TAG_SOURCES = $(shell find -L $(SRCDIR)/base $(SRCDIR)/tag -name '*.cpp')
TAG_BIN = tag 

LASER_TAG_SOURCES = $(shell find -L $(SRCDIR)/base $(SRCDIR)/laser_tag -name '*.cpp')
LASER_TAG_BIN = lasertag 

NOISY_TAG_SOURCES = $(shell find -L $(SRCDIR)/base $(SRCDIR)/noisy_laser_tag -name '*.cpp')
NOISY_TAG_BIN = noisytag 


# -------
# Targets
# -------

.PHONY: all clean 

all: TAG_BIN LASER_TAG_BIN NOISY_TAG_BIN

TAG_BIN:
	@cd $(DESPOTHOME) && make
	$(CXX) $(LDFLAGS) $(LDFLAGS) $(TAG_SOURCES) $(INCLUDE) $(LIBDIR)/*.o -o $(TAG_BIN)

LASER_TAG_BIN:
	@cd $(DESPOTHOME) && make
	$(CXX) $(LDFLAGS) $(LDFLAGS) $(LASER_TAG_SOURCES) $(INCLUDE) $(LIBDIR)/*.o -o $(LASER_TAG_BIN)

NOISY_TAG_BIN:
	@cd $(DESPOTHOME) && make
	$(CXX) $(LDFLAGS) $(LDFLAGS) $(NOISY_TAG_SOURCES) $(INCLUDE) $(LIBDIR)/*.o -o $(NOISY_TAG_BIN)

TAG_BIN_LIGHT:
	@cd $(DESPOTHOME) && make
	$(CXX) $(LDFLAGS) $(TAG_SOURCES) $(INCLUDE) -L $(LIBDIR) -ldespot -o $(TAG_BIN)

LASER_TAG_BIN_LIGHT:
	@cd $(DESPOTHOME) && make
	$(CXX) $(LDFLAGS) $(LASER_TAG_SOURCES) $(INCLUDE) -L $(LIBDIR) -ldespot -o $(LASER_TAG_BIN)

clean:
	rm -f $(TAG_BIN) $(LASER_TAG_BIN)
