# Setup Guide

## Dependencies

The following sections list the dependencies and how to obtain them.

> [!IMPORTANT]
> If you obtain a dependency with `git clone`, it is strongly recommended to checkout a recent tag,
> rather than using the most recent version on the main branch. For example, to use version `10.2.0`
> of `fmt`, run (while in the `fmt` repository directory):
> ```bash
> git checkout 10.2.0
> ```
> See the list of tags in chronological order (latest is first) by running:
> ```bash
> git log --tags --decorate --simplify-by-decoration --oneline
> ```

### :large_orange_diamond: `meson`: Build system used by `iguana`
<https://mesonbuild.com/>
- likely available in your package manager
- you may also install `meson` (and `ninja`) with `pip`:
```bash
python -m pip install meson ninja
```

### :large_orange_diamond: `fmt`: C++ output formatting library
<https://github.com/fmtlib/fmt>
- likely available in your package manager, possibly under `libfmt`
- if you compile it yourself, include the `cmake` option `-DCMAKE_POSITION_INDEPENDENT_CODE=ON`
- example `cmake` commands:
```bash
cmake -S /path/to/fmt_source_code -B build-fmt -DCMAKE_INSTALL_PREFIX=/path/to/fmt_installation -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build-fmt -j$(nproc)
cmake --install build-fmt
```

### :large_orange_diamond: `hipo`: C++ HIPO API
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
./configure.py --hipo /path/to/hipo_installation --fmt /path/to/fmt_installation
```
This will generate a configuration file (`build-iguana.ini`, by default) with the build settings, along with an installation script (`install-iguana.sh`).
Inspect both of them, and if they look correct, proceed with building and installing `iguana` by running:
```bash
./install-iguana.sh
```

> [!TIP]
> You may edit the configuration file (`build-iguana.ini`, by default) to
> change any settings, and re-build `iguana` with `./install-iguana.sh`.
> - this procedure is preferred if you just want to change some settings
> - re-running `configure.py` will _overwrite_ your configuration file

> [!TIP]
> If you have trouble and want to try a clean build, wipe your build directory (`build-iguana/`, by default). The safest
> way to do this is:
> ```bash
> meson setup --wipe build-iguana
> ```

### Using Meson Directly

Instead of `configure.py`, use `meson` directly for more control:

1. Follow the [note on dependency resolution](dependency_resolution.md)
2. Build with `meson`, for example
```bash
meson setup --prefix=$(pwd)/iguana build-iguana /path/to/iguana/repository
meson install -C build-iguana
```

> [!TIP]
> `configure.py` produces a native file (`.ini`) which may be used by `meson setup` option `--native-file`.
