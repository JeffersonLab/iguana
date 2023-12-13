# compiler and flags
CXX := g++

# dependency libraries
DEP_LIBRARIES = $(shell pkg-config --libs iguana)
DEP_LIBRARIES += $(shell pkg-config --libs fmt)
DEP_LIBRARIES += -L${HIPO}/lib -lhipo4

# dependency headers
DEP_INCLUDES = $(shell pkg-config --cflags iguana)
DEP_INCLUDES += $(shell pkg-config --cflags fmt)
DEP_INCLUDES += -I${HIPO}/include

# assume each .cc file has `main`; install it to ./bin/
BINDIR = bin
EXECUTABLES := $(addprefix $(BINDIR)/, $(basename $(wildcard *.cc)))

# build executable
# - you may want to set the rpath here too
$(EXECUTABLES): $(BINDIR)/%: %.cc
	mkdir -p $(BINDIR)
	$(CXX) -c $< -o $@.o $(DEP_INCLUDES)
	$(CXX) -o $@ $@.o $(DEP_LIBRARIES)
	$(RM) $@.o
