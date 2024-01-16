# Setup Guide

| **Table of Contents** |
| --- |
| [Dependencies](#dependencies) |
| [Building and Installing](#building) |
| [Environment Variables (optional)](#env) |

<a name="dependencies"></a>
## Dependencies

The following sections list the dependencies and how to obtain them.

> [!TIP]
> - It's generally better to use your a package manager to install dependencies, _e.g._:
>   - `brew install <package>` macOS
>   - `apt install <package>`, `dnf install <package>`, `pacman -S <package` on Linux
>   - The name of the package may be different for different package managers; search for and read about the package before installing it
> - If you obtain a dependency from Github (or similar), it's best practice to obtain a recent tag rather than the latest version on the main branch:
>   ```
>   git log --tags --decorate --simplify-by-decoration --oneline     # list all the tags (latest first)
>   git checkout 1.0.0                                               # checkout the tag '1.0.0'
>   ```

### :large_orange_diamond: `meson`: Build system used by `iguana`
<https://mesonbuild.com/>
- Likely available in your package manager, but the latest version is preferred and may be installed with `pip`:
```bash
python -m pip install meson ninja
```
This includes [`ninja`](https://ninja-build.org/), which `meson` will benefit from using. 

### :large_orange_diamond: `fmt`: C++ output formatting library
<https://github.com/fmtlib/fmt>
- Likely available in your package manager, likely as `fmt` or `libfmt`
  - If you need Python bindings on macOS, please install `fmt` with `brew install fmt`
  - If you compile it yourself on Linux, include the `cmake` option `-DCMAKE_POSITION_INDEPENDENT_CODE=ON` to build the static library

### :large_orange_diamond: `hipo`: C++ HIPO API
<https://github.com/gavalian/hipo>
- Use the `hipo` module on `ifarm`, or obtain and build it yourself
- Example `cmake` commands:
```bash
cmake -S /path/to/hipo_source_code -B build-hipo -DCMAKE_INSTALL_PREFIX=/path/to/hipo_installation
cmake --build build-hipo -j$(nproc)
cmake --install build-hipo
```

<a name="building"></a>
## Building and Installing

- For convenience, a configuration script is provided.
- Advanced users who want more control may skip to the "Using Meson Directly" section.

### :large_blue_diamond: Using the Configuration Script

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
> change any settings, and rebuild `iguana` with `./install-iguana.sh`.
> - this procedure is preferred if you just want to change some settings
> - re-running `configure.py` will _overwrite_ your configuration file

> [!TIP]
> If you have trouble and want to try a clean build, wipe your build directory (`build-iguana/`, by default). The safest
> way to do this is:
> ```bash
> meson setup --wipe build-iguana
> ```
> Then try to rebuild

### :large_blue_diamond: Using Meson Directly

Instead of `configure.py`, use `meson` directly for more control:

1. Follow the [note on dependency resolution](dependency_resolution.md)
2. Build with `meson`, for example
```bash
meson setup --prefix=$(pwd)/iguana build-iguana /path/to/iguana/repository
meson install -C build-iguana
```

> [!TIP]
> `configure.py` produces a native file (`.ini`) which may be used by `meson setup` option `--native-file`.


<a name="env"></a>
## Environment Variables (optional)
The C++ `iguana` implementation does not require the use of any environment variables. However,
- some language bindings may benefit from variables such as `$PYTHONPATH`, for Python
- you may want to override the linker library search path list (_e.g._, if you have conflicting libraries in it)

You may set your own environment variables, but for a quick start with suggested settings,
the installed file `bin/this_iguana.sh` may be used as
```
source bin/this_iguana.sh [OPTIONAL ARGUMENTS]...

OPTIONAL ARGUMENTS:

   ld       append library paths to LD_LIBRARY_PATH (or DYLD_LIBRARY_PATH);
            by default these variables are NOT modified

   verbose  print the relevant environment variable values
```

which sets or modifies the following environment variables:

| Variable                                                 | Modification                                                                                                                              |
| ---                                                      | ---                                                                                                                                       |
| `PKG_CONFIG_PATH`                                        | adds paths to the `pkg-config` files (`.pc`) for dependencies and `iguana`; see [note on dependency resolution](dependency_resolution.md) |
| `PYTHONPATH`                                             | adds paths to dependency and `iguana` Python packages, if Python bindings are installed                                                   |
| `LD_LIBRARY_PATH` (Linux) or `DYLD_LIBRARY_PATH` (macOS) | adds paths to dependency and `iguana` libraries, if the optional argument `ld` was used                                                   |

`this_iguana.sh` is compatible with `bash` and `zsh`, but not with `tcsh` or `csh`.
