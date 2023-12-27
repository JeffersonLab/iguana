# compiler
CXX := g++
CFLAGS := -std=c++17

# iguana headers and libraries
IGUANA_INCLUDES  := $(shell pkg-config --cflags iguana)
IGUANA_LIBRARIES := $(shell pkg-config --libs   iguana)

# set rpath
# - this is so that the executable knows where the dependency libraries are
# - alternatively, set $LD_LIBRARY_PATH before running your executables
get_libdir = -rpath=$(shell pkg-config --variable libdir $1)
LDFLAGS := -Wl,$(call get_libdir, iguana),$(call get_libdir, hipo4)

# installation directory
BINDIR := bin

# assume we want each `.cc` file built as an executable
EXECUTABLES := $(addprefix $(BINDIR)/, $(basename $(wildcard *.cc)))

# build each executable and install to $(BINDIR)
$(EXECUTABLES): $(BINDIR)/%: %.cc
	mkdir -p $(BINDIR)
	$(CXX) -c $< -o $@.o $(CFLAGS) $(IGUANA_INCLUDES)
	$(CXX) -o $@ $@.o $(LDFLAGS) $(IGUANA_LIBRARIES)
	$(RM) $@.o

# remove the executable
clean:
	$(RM) $(EXECUTABLES)
