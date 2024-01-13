# Dependency Resolution

The dependencies in [the setup guide](setup.md) must be locatable by `iguana`, or
by anything that depends on `iguana`.

Assuming a dependency is installed in `$prefix`, it uses one or more of the following:
- `pkg-config`: a `.pc` file in `$prefix/lib/pkgconfig`
- `.cmake` files in `$prefix/lib/cmake`

Take a look at each dependency's installation prefix to see which of these options
are available.

## Resolving Build Dependencies

To use these dependencies with your software, they must be findable by its build system
The following sections explain how to do so with each.

### :large_blue_diamond: Meson
For `iguana`, the build system is `meson`, which accepts the build options
```bash
-Dpkg_config_path=$prefix/lib/pkgconfig
-Dcmake_prefix_path=$prefix
```
(where multiple paths are delimited by commas).

### :large_blue_diamond: CMake
For `cmake`, the `pkg-config` path can be combined with the `cmake` path, so only the
build option
```bash
-DCMAKE_PREFIX_PATH="$prefix"
```
is needed; this assumes:
- all dependencies are in `$prefix` (delimit multiple paths with semicolons)
- `PKG_CONFIG_USE_CMAKE_PREFIX_PATH` has not been disabled.

### :large_blue_diamond: General Case
Environment variables may be used instead of build options for a general approach:
```bash
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig
export CMAKE_PREFIX_PATH=$prefix
```
(where multiple paths are delimited by colons).

`pkg-config` files (`.pc`) allow for usage of the `pkg-config` command. Assuming the package is `hipo4` and `hipo4.pc` is found in `PKG_CONFIG_PATH`, compiler flags may be found by
```bash
pkg-config --libs hipo4
pkg-config --cflags hipo4
```
Any variable defined in `hipo4.pc` is accessible with `pkg-config --variable <variable>`.

## Resolving Runtime Dependencies

TODO: change these notes into documentation

- prefer `rpath`s over `$LD_LIBRARY_PATH` or `$DYLD_LIBRARY_PATH` (or other
  environment variables), since they are globally mutable
- both `rpaths` and environment variables are OS-dependent
- while `rpaths` are typical in build files, they are usually stripped at
installation, otherwise dependencies cannot be relocated (easily)
- containerization with one installation prefix removes the need of both
  `rpath`s and `ld` environment vars since dependency files will all be findable in the
  default, assumed places
