cmake_minimum_required(VERSION 3.6)

project(
  iguana-build-example
  LANGUAGES CXX
  VERSION 1.0.0
)

# find dependencies
# - iguana doesn't create a `iguanaConfig.cmake` file, but it does create a pkg-config `.pc` file;
#   the target will be imported as `PkgConfig::iguana`
find_package(hipo4 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(iguana REQUIRED IMPORTED_TARGET iguana)

# set the rpath to use the link path
# - this is so that the executable knows where the dependency libraries are
# - alternatively, set $LD_LIBRARY_PATH before running your executables
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# build and install the executable
set(EXAMPLE_BIN iguana-example-00-basic)
add_executable(${EXAMPLE_BIN})
target_sources(${EXAMPLE_BIN} PRIVATE ${EXAMPLE_BIN}.cc)
target_link_libraries(${EXAMPLE_BIN} PUBLIC PkgConfig::iguana hipo4)
install(TARGETS ${EXAMPLE_BIN})
