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
> - If you obtain a dependency from GitHub (or similar), it's best practice to obtain a recent tag rather than the latest version on the main branch:
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

Iguana uses [`meson`](https://mesonbuild.com/) as its build system. From here, we assume that:
- you are in a working directory, which may be any directory
- the Iguana source code directory (this repository) is found at `/path/to/iguana-source`

### Step 1. Resolve Dependencies

Any dependencies which are not installed in the system-wide default locations will need to be found.
Use [`meson/resolve-dependencies.py`](meson/resolve-dependencies.py) to help you:
```bash
/path/to/iguana-source/meson/resolve-dependencies.py --help    # prints the usage guide
```
Tell it where your dependencies are installed and it will tell you the build options
that you need for Step 2; you can also choose to write those build options to an INI (native) file.

See also the [note on dependency resolution](dependency_resolution.md) for more general guidance.


### Step 2. Generate a build directory

Make a build directory named `build-iguana` (you may choose any name):
```bash
meson setup build-iguana /path/to/iguana-source [BUILD_OPTIONS_FROM_STEP_1]
```
You'll need to replace `[BUILD_OPTIONS_FROM_STEP_1]` with the build options from Step 1 above.

### Step 3. Set build options

Now let's configure the build directory (assuming its `./build-iguana` here).
If you will _install_ `iguana` (recommended), set an installation prefix:
```bash
meson configure --prefix=/path/to/iguana-installation build-iguana
```
The path must be _absolute_; if you want it relative to your current working directory, _e.g. `./iguana`, use `--prefix=$(pwd)/iguana`

Additional build options and their descriptions may be found by running
```bash
meson configure /path/to/iguana
```
**That's a _lot_ of text!** However, the most important build options are near the bottom, under "Project options". For example, to
enable building of Iguana examples, run
```bash
meson configure -Dexamples=true build-iguana
```
You can add as many `-D<option>=value` arguments as you need.

> [!TIP]
> You can see all of your options' values in the build directory `build-iguana` by running:
> ```bash
> meson configure build-iguana
> ```

> [!TIP]
> You can also set all of your build options in Step 2, since `meson setup` accepts similar build-options arguments.

### Step 4. Compile and Install
Now compile and install Iguana:
```bash
meson compile -C build-iguana   # builds Iguana, filling your build directory
meson install -C build-iguana   # installs Iguana to your prefix (build option 'prefix')
```

> [!TIP]
> You can use combine these two commands by just running the second "`install`" one

> [!IMPORTANT]
> If you have trouble and want to try a clean build, do _not_ delete your build directory (since you spent the time to configure it);
> instead, clean your build directory by running:
> ```bash
> meson setup --wipe build-iguana /path/to/iguana-source
> ```
> This will preserve your build options; then try to rebuild.


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
