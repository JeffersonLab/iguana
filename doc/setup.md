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
>   - macOS Homebrew: `brew install <package>`
>   - Linux (depends on distribution) examples: `apt install <package>`, `dnf install <package>`, `pacman -S <package>`
>   - The name of the package may be different for different package managers; search for and read about the package before installing it
> - If you obtain a dependency from Github (or similar), it's best practice to obtain a recent tag rather than the latest version on the main branch:
>   ```
>   git log --tags --decorate --simplify-by-decoration --oneline     # list all the tags (latest first)
>   git checkout 1.0.0                                               # checkout the tag '1.0.0'
>   ```

### :large_orange_diamond: `meson`: Build system used by Iguana
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

Iguana uses [`meson`](https://mesonbuild.com/) as its build system. From here,
we'll assume that you are in a working directory, which may be any directory.
- it does not have to be your analysis working directory, since we are only
_building_ Iguana at this time
- you may also just use the top-level source code directory (`.`)

We'll also assume the Iguana source code directory (this repository) is found at `/path/to/iguana`

Follow guidance in the next 3 steps below; as a quick reference, here are the commands you'll run:
```bash
meson setup --native-file <INI_file> <build_directory> <source_directory>
meson install -C <build_directory>
```

### 1. Set build options
It's recommended to create an INI file to set the build options that you want
to use. Copy the example INI file [`meson/build-iguana.ini`](meson/build-iguana.ini) to
your working directory:
```bash
cp /path/to/iguana/meson/build-iguana.ini ./my-iguana.ini    # you may choose any file name
```
Then edit it, setting your preferred options; in particular, you'll need to set
the dependency resolution options. Use [`meson/resolve-dependencies.py`](meson/resolve-dependencies.py)
to help you:
```bash
/path/to/iguana/meson/resolve-dependencies.py --help    # prints the usage guide
```
See the [note on dependency resolution](dependency_resolution.md) for more guidance.

### 2.Configure your build
Next, start an Iguana build directory; let's call it `build-iguana` (you may choose any name):
```bash
meson setup --native-file my-iguana.ini build-iguana /path/to/iguana
```

> [!TIP]
> If you _already_ have an Iguana build directory, and you have just changed options in your INI file, add the `--reconfigure` option.
> Alternatively, run `meson configure build-iguana -D<option>=<value>`

> [!NOTE]
> If you did not set `prefix` in your INI file, you can set it with the `--prefix` option of
> `meson setup` or `meson configure`
> - If you do _not_ set `prefix`, it will default to a system installation.
> - The prefix must be an absolute path; if you want it to be relative to your working directory, _e.g._ `./iguana/`, set it to `$(pwd)/iguana`

### 3. Compile and Install
Now compile and install Iguana:
```bash
meson install -C build-iguana
```
> [!TIP]
> You can use `meson compile` and `meson install`, if you want to separate compiling and installing
> (_e.g._, if you do only want to build, but not install Iguana).

> [!TIP]
> If you have trouble and want to try a clean build, wipe your build directory by running:
> ```bash
> meson setup --wipe build-iguana
> ```
> Then try to rebuild


<a name="env"></a>
## Environment Variables (optional)
The C++ Iguana implementation does not require the use of any environment variables. However,
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
| `PKG_CONFIG_PATH`                                        | adds paths to the `pkg-config` files (`.pc`) for dependencies and Iguana; see [note on dependency resolution](dependency_resolution.md)   |
| `PYTHONPATH`                                             | adds paths to dependency and Iguana Python packages, if Python bindings are installed                                                     |
| `LD_LIBRARY_PATH` (Linux) or `DYLD_LIBRARY_PATH` (macOS) | adds paths to dependency and Iguana libraries, if the optional argument `ld` was used                                                     |

`this_iguana.sh` is compatible with `bash` and `zsh`, but not with `tcsh` or `csh`.
