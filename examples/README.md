# Examples

All of the examples (`*.cc`) in [this `examples/` directory](/examples) may be built if the Iguana build
option `examples` is set to `True`. They will be installed in the `bin/`
subdirectory of your `iguana` installation.

If you would like to integrate `iguana` into an existing analysis, you'll need
to add `iguana` as a dependency. The following subdirectories are
standalone examples demonstrating how to do this, by showing how to build an
executable that depends on `iguana` libraries:
- [`build_with_cmake/`](build_with_cmake) - CMake integration
- [`build_with_meson/`](build_with_meson) - Meson integration
- [`build_with_make/`](build_with_make) - Makefile integration
