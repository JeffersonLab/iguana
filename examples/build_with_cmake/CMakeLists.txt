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
find_package(PkgConfig REQUIRED)
pkg_check_modules(hipo4 REQUIRED IMPORTED_TARGET hipo4)
pkg_check_modules(iguana REQUIRED IMPORTED_TARGET iguana)

# build and install the executable
set(EXAMPLE_BIN iguana-example-00-basic)
add_executable(${EXAMPLE_BIN})
target_sources(${EXAMPLE_BIN} PRIVATE ${EXAMPLE_BIN}.cc)
target_link_libraries(${EXAMPLE_BIN} PUBLIC PkgConfig::iguana PkgConfig::hipo4)
install(TARGETS ${EXAMPLE_BIN})
