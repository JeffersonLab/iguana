# Dependency Resolution

The dependencies in [the setup guide](setup.md) must be locatable by `iguana`, or
by anything that depends on `iguana`.

Assuming a dependency is installed in `$prefix`, it uses one or more of the following:
- `pkg-config`: a `.pc` file in `$prefix/lib/pkgconfig`
- `.cmake` files in `$prefix/lib/cmake`

Take a look at each dependency's installation prefix to see which of these options
are available.

One way of using these dependencies is with environment variables
(multiple paths are delimited by colons):
- `pkg-config`: include `$prefix/lib/pkgconfig` in `$PKG_CONFIG_PATH`
- `cmake`: include `$prefix` in `$CMAKE_PREFIX_PATH`

A better way is to use build options; for `meson`, use the following
(multiple paths are delimited by commas):
- `-Dpkg_config_path`
- `-Dcmake_prefix_path`
