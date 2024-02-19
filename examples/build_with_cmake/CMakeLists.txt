cmake_minimum_required(VERSION 3.6)

project(
  iguana-build-example
  LANGUAGES CXX
  VERSION 1.0.0
)

# set C++ standard
if(NOT "${CMAKE_CXX_STANDARD}")
  set(CMAKE_CXX_STANDARD 17)
endif()

# find dependencies
# - using `PkgConfig` module imports pkg-config dependencies as targets prefixed with `PkgConfig::`
# - if iguana was built with `static_hipo=True`, the `hipo4` target is not needed here
find_package(PkgConfig REQUIRED)
pkg_check_modules(iguana REQUIRED IMPORTED_TARGET iguana)

# set the rpath to use the link path
# - this is so that the executable knows where the dependency libraries are
# - alternatively, set $LD_LIBRARY_PATH before running your executables ($DYLD_LIBRARY_PATH on macOS)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# build and install the executable
set(EXAMPLE_BIN iguana-example-00-basic)
add_executable(${EXAMPLE_BIN})
target_sources(${EXAMPLE_BIN} PRIVATE ${EXAMPLE_BIN}.cc)
target_link_libraries(${EXAMPLE_BIN} PUBLIC PkgConfig::iguana)
install(TARGETS ${EXAMPLE_BIN})
