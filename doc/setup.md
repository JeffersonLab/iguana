# Setup Guide

## Dependencies

### `meson`: Build system used by `iguana`
<https://mesonbuild.com/>
- likely available in your package manager
- you may also install `meson` (and `ninja`) with `pip`:
```bash
python -m pip install meson ninja
```

### `fmt`: C++ output formatting library
<https://github.com/fmtlib/fmt>
- likely available in your package manager, possibly under `libfmt`
- if you compile it yourself, include the `cmake` option `-DCMAKE_POSITION_INDEPENDENT_CODE=ON`
- example `cmake` commands:
```bash
cmake -S /path/to/fmt_source_code -B build-fmt -DCMAKE_INSTALL_PREFIX=/path/to/fmt_installation -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build-fmt -j$(nproc)
cmake --install build-fmt
```

### `hipo`: C++ HIPO API
<https://github.com/gavalian/hipo>
- use the `hipo` module on `ifarm`, or obtain and build it yourself
- example `cmake` commands:
```bash
cmake -S /path/to/hipo_source_code -B build-hipo -DCMAKE_INSTALL_PREFIX=/path/to/hipo_installation
cmake --build build-hipo -j$(nproc)
cmake --install build-hipo
```

## Building and Installing

- For convenience, a configuration script is provided.
- Advanced users who want more control may skip to the "Using Meson Directly" section.

### Using the Configuration Script

First, configure your `iguana` build using `configure.py`:
```bash
./configure.py --help
```
The `--help` option will print the usage guide.
Unless the dependencies are installed in one of the system default locations, you will need to specify the path to each of them, _e.g._,
```bash
./configure.py --hipo /path/to/hipo_installation
```
This will generate a configuration file (`.ini`) with the build settings, along with an installation script (`install-iguana.sh`).
Inspect both of them, and if they look correct, proceed with building and installing `iguana` by running:
```bash
./install-iguana.sh
```

### Using Meson Directly

Instead of `configure.py`, use `meson` directly for more control:

1. The version number is dynamically determined from the `git` tag; either:
   - run `.github/detect-version.sh`
   - put the version number in `.version` in the top-level directory of the repository
2. Follow the [note on dependency resolution](dependency_resolution.md)
3. Build with `meson`, for example
```bash
meson setup --prefix=$(pwd)/iguana build-iguana /path/to/iguana/repository
meson install -C build-iguana
```
**NOTE**: `configure.py` produces a native file (`.ini`) which may be used by
`meson setup` option `--native-file`.
