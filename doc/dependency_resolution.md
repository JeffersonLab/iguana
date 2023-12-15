# Dependency Resolution

The dependencies in [the setup guide](setup.md) must be locatable by `iguana`, or
by anything that depends on `iguana`.

Assuming a dependency is installed in `$prefix`, it uses one or more of the following:
- `pkg-config`: a `.pc` file in `$prefix/lib/pkgconfig`
- `.cmake` files in `$prefix/lib/cmake`

Take a look at each dependency's installation prefix to see which of these options
are available.

To use these dependencies with your software, they must be findable by its build system.
The following sections explain how to do so with each.

### Meson
For `iguana`, the build system is `meson`, which accepts the build options
```bash
-Dpkg_config_path=$prefix/lib/pkgconfig
-Dcmake_prefix_path=$prefix
```
(where multiple paths are delimited by commas).

### CMake
For `cmake`, the `pkg-config` path can be combined with the `cmake` path, so only the
build option
```bash
-DCMAKE_PREFIX_PATH="$prefix"
```
is needed; this assumes:
- all dependencies are in `$prefix` (delimit multiple paths with semicolons)
- `PKG_CONFIG_USE_CMAKE_PREFIX_PATH` has not been disabled.

### General Case
Environment variables may be used instead of build options for a general approach:
```bash
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig
export CMAKE_PREFIX_PATH=$prefix
```
(where multiple paths are delimited by colons).
