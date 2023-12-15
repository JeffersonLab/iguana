# Examples

All of the examples (`*.cc`) in this directory may be built using the
`--examples` option of `configure.py`; they will be installed in the `bin/`
subdirectory of your `iguana` installation.

If you would like to integrate `iguana` into an existing analysis, you'll need
to add `iguana` as a dependency. The `build_with_*/` subdirectories are
standalone examples demonstrating how to do this, by showing how to build an
executable that depends on `iguana` libraries. Each subdirectory is for a
different build system tool.
