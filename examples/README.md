# Examples

All of the examples (`*.cc`) in [this `examples/` directory](/examples) may be built if the Iguana build
option `install_examples` is set to `True`. They will be installed in the `bin/`
subdirectory of your Iguana installation. Read the example code to understand how to use Iguana, or

- [View the Iguana User's Guide](https://jeffersonlab.github.io/iguana/doxygen)

If you would like to integrate Iguana into an existing analysis, you'll need
to add Iguana as a dependency. The following subdirectories are
standalone examples demonstrating how to do this, by showing how to build an
executable that depends on Iguana libraries:
- [`build_with_cmake/`](build_with_cmake) - CMake integration
- [`build_with_meson/`](build_with_meson) - Meson integration
- [`build_with_make/`](build_with_make) - Makefile integration
