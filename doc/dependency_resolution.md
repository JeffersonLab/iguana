# Dependency Resolution

The dependencies in [the setup guide](setup.md) must be locatable by `iguana`, or
by anything that depends on `iguana`.

Assuming a dependency is installed in `$prefix`, it uses one or more of the following:
- `pkg-config`: a `.pc` file in `$prefix/lib/pkgconfig`
- `.cmake` files in `$prefix/lib/cmake`

Take a look at each dependency's installation prefix to see which of these options
are available.

One way of using these dependencies is with environment variables:
```bash
export PKG_CONFIG_PATH=$prefix/lib/pkgconfig
export CMAKE_PREFIX_PATH=$prefix
```
Multiple paths are delimited by colons.

A better way is to use build options; for `meson`, use the following:
```bash
-Dpkg_config_path=$prefix/lib/pkgconfig
-Dcmake_prefix_path=$prefix
```
Multiple paths are delimited by commas.
