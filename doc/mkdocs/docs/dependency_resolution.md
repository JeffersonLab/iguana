# Dependency Resolution

The dependencies in [the setup guide](setup.md) must be locatable by `iguana`, or
by anything that depends on `iguana`.

Assuming a dependency is installed in `$prefix`, it uses one or more of the following:

- `pkg-config`: a `.pc` file in `$prefix/lib/pkgconfig`
- `.cmake` files in `$prefix/lib/cmake`

Take a look at each dependency's installation prefix to see which of these options
are available.

!!! note
    Some installations will have libraries, and therefore `cmake/` and `pkgconfig/` directories, within
    `$prefix/lib64`, or within `$prefix/lib/x86_64-linux-gnu`. For brevity in the documentation below, we assume
    they are in `$prefix/lib`.

## Resolving Build Dependencies

To use these dependencies with your software, they must be findable by its build system.
The following sections explain how to do so with each.

### 🔷 Meson
For `iguana`, the build system is `meson`, which accepts the build options
``` bash
-Dpkg_config_path=$prefix/lib/pkgconfig
-Dcmake_prefix_path=$prefix
```
(where multiple paths are delimited by commas).

### 🔷 CMake
For `cmake`, the `pkg-config` path can be combined with the `cmake` path, so only the
build option
``` bash
-DCMAKE_PREFIX_PATH="$prefix"
```
is needed; this assumes:

- all dependencies are in `$prefix` (delimit multiple paths with semicolons)
- `PKG_CONFIG_USE_CMAKE_PREFIX_PATH` has not been disabled.

### 🔷 General Case
Environment variables may be used instead of build options for a general approach:
``` bash
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig
export CMAKE_PREFIX_PATH=$prefix
```
(where multiple paths are delimited by colons).

`pkg-config` files (`.pc`) allow for usage of the `pkg-config` command. Assuming the package is `hipo4` and `hipo4.pc` is found in `PKG_CONFIG_PATH`, compiler flags may be found by
``` bash
pkg-config --libs hipo4
pkg-config --cflags hipo4
```
Any variable defined in `hipo4.pc` is accessible with `pkg-config --variable <variable>`.

## Resolving Runtime Dependencies

Depending on your local setup and the current state of your environment variables, you may need to set some variables
such that `iguana` is prioritized. See [the Environment Variables section in the setup guide for more details](setup.md#env).
