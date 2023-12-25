# compiler
CXX := g++

# dependency libraries
DEP_LIBRARIES = $(shell pkg-config --libs iguana)
DEP_LIBRARIES += -lhipo4

# dependency headers
DEP_INCLUDES = $(shell pkg-config --cflags iguana)
DEP_INCLUDES += -I${HIPO}/include

# assume each .cc file has `main`; install it to ./bin/
BINDIR = bin
EXECUTABLES := $(addprefix $(BINDIR)/, $(basename $(wildcard *.cc)))

# build executable
# - NOTE: we haven't set an rpath here, so the user will need
#   to make sure dependency libraries are found in $LD_LIBRARY_PATH
$(EXECUTABLES): $(BINDIR)/%: %.cc
	mkdir -p $(BINDIR)
	$(CXX) -c $< -o $@.o $(DEP_INCLUDES)
	$(CXX) -o $@ $@.o $(DEP_LIBRARIES)
	$(RM) $@.o
