# Setup Guide

| **Table of Contents**                           |
| ---                                             |
| 🟠 [**Dependencies**](#dependencies)            |
| 🟠 [**Building and Installing**](#building)     |
| 🟠 [**Environment Variables** (optional)](#env) |

<a name="dependencies"></a>
## 🟠 Dependencies

The following sections (🔶) list the dependencies and how to obtain them.

> [!TIP]
> It's generally better to use your a package manager to install most dependencies, _e.g._:
> - macOS Homebrew: `brew install <package>`
> - Linux (depends on distribution) examples: `apt install <package>`, `dnf install <package>`, `pacman -S <package>`
> - The name of the package may be different for different package managers; search for and read about the package before installing it

> [!IMPORTANT]
> If you obtain a dependency from GitHub (or similar), it's best practice to obtain a recent tag rather than the latest version on the main branch:
> ```
> git log --tags --decorate --simplify-by-decoration --oneline     # list all the tags (latest first)
> git checkout 1.0.0                                               # checkout the tag '1.0.0'
> ```

### 🔶 `meson`: Build system used by Iguana
<https://mesonbuild.com/>
- Likely available in your package manager, but the latest version is preferred and may be installed with `pip`:
```bash
python -m pip install meson ninja
```
This includes [`ninja`](https://ninja-build.org/), which `meson` will benefit from using. 

### 🔶 `fmt`: C++ output formatting library
<https://github.com/fmtlib/fmt>
- Likely available in your package manager, likely as `fmt` or `libfmt`
  - If you need Python bindings on macOS, please install `fmt` with `brew install fmt`
  - If you compile it yourself on Linux, include the `cmake` option `-DCMAKE_POSITION_INDEPENDENT_CODE=ON` to build the static library

### 🔶 `yaml-cpp`: YAML parser and emitter
<https://github.com/jbeder/yaml-cpp>
- Likely available in your package manager, likely as `yaml-cpp`

### 🔶 `hipo`: C++ HIPO API
<https://github.com/gavalian/hipo>
- Use the `hipo` module on `ifarm`, or obtain and build it yourself
- Example `cmake` commands:
```bash
cmake -S /path/to/hipo_source_code -B build-hipo -DCMAKE_INSTALL_PREFIX=/path/to/hipo_installation
cmake --build build-hipo
cmake --install build-hipo
```

### 🔶 Optional: `ROOT`: Data analysis framework
<https://root.cern.ch/>
- ROOT is an **optional** dependency: some algorithms and test code depends on ROOT, but if you do not
  have ROOT on your system, `iguana` will build everything _except_ ROOT-dependent code
- It is **NOT recommended** to use your package manager to install ROOT; the most reliable installation
  method is [building it from source](https://root.cern/install/build_from_source/)
  - You may need to set the C++ standard to match that used in `iguana`, which is currently 17; to do so,
    use the build option `-DCMAKE_CXX_STANDARD=17`
- After installation, depending on ROOT's installation prefix you may also need to set your environment so
  ROOT may be found; this is typically done by `source /path/to/root/bin/thisroot.sh`

<a name="building"></a>
## 🟠 Building and Installing

Iguana uses [`meson`](https://mesonbuild.com/) as its build system. From here, we assume that:
- you are in a working directory, which may be any directory
- the Iguana source code directory (this repository) is found at `/path/to/iguana-source`

The following Steps (🟩) explain how to use `meson` to install Iguana.

### 🟩 Step 1: Resolve Dependencies

Any dependencies which are not installed in the system-wide default locations will need to be found.
Use [`meson/resolve-dependencies.py`](../meson/resolve-dependencies.py) to help you:
```bash
/path/to/iguana-source/meson/resolve-dependencies.py --help    # prints the usage guide
```
Tell it where your dependencies are installed and it will tell you the build options
that you need for Step 2; you can also choose to write those build options to an INI (native) file.

Alternatively, you may use environment variables; see the [note on dependency
resolution](dependency_resolution.md) for more general guidance.


### 🟩 Step 2: Generate a build directory

Make a build directory, then `cd` into it. You may choose any name, but we'll use `build-iguana` in this example:
```bash
meson setup build-iguana /path/to/iguana-source [BUILD_OPTIONS_FROM_STEP_1]
cd build-iguana
```
You'll need to replace `[BUILD_OPTIONS_FROM_STEP_1]` with the build options from Step 1 above.

> [!IMPORTANT]
> The next steps assume your current directory is the build directory. Refer to `meson` documentation if
> you'd rather be in a different directory.

### 🟩 Step 3: Set build options

If you will _install_ `iguana` (recommended), set an installation prefix:
```bash
meson configure --prefix=/path/to/iguana-installation  # must be an ABSOLUTE path
```

All build options, their current values, and their descriptions may be found by running
```bash
meson configure
```
**but that's a _lot_ of text!** The _most important_ build options are near the bottom, under **"Project options"**.

To set any build option, _e.g._ `examples` to `true` (enables building of Iguana examples), run:
```bash
meson configure -Dexamples=true
```
You can add as many `-D<option>=<value>` arguments as you need.

### 🟩 Step 4: Compile and Install
Now compile and install Iguana:
```bash
meson compile   # builds Iguana, filling your build directory
meson install   # installs Iguana to your prefix (build option 'prefix')
```

> [!TIP]
> You can use combine these two commands by just running `meson install`

> [!IMPORTANT]
> If you have trouble and want to try a clean build, do _not_ delete your build directory (since you spent the time to configure it);
> instead, clean your build directory by running:
> ```bash
> meson setup --wipe /path/to/iguana-source
> ```
> This will preserve your build options; then try to rebuild.


<a name="env"></a>
## 🟠 Environment Variables (optional)
The C++ Iguana implementation does not require the use of any environment variables. However,
- if Iguana libraries are not in your default linker library search path, you may need to update it, _e.g._ with
  `$LD_LIBRARY_PATH` (Linux) or `$DYLD_LIBRARY_PATH` (macOS)
- some language bindings may need variables such as `$PYTHONPATH`, for Python

You may set your own environment variables, but for a quick start with suggested settings,
the installed file `bin/this_iguana.sh` may be used as
```
source bin/this_iguana.sh
```
Use the `--help` argument to see its full usage guide.

The following environment variables are set or modified:

| Variable                                                 | Modification                                                                                                                              |
| ---                                                      | ---                                                                                                                                       |
| `PKG_CONFIG_PATH`                                        | adds paths to the `pkg-config` files (`.pc`) for dependencies and Iguana; see [note on dependency resolution](dependency_resolution.md)   |
| `LD_LIBRARY_PATH` (Linux) or `DYLD_LIBRARY_PATH` (macOS) | adds paths to dependency and Iguana libraries                                                                                             |
| `PYTHONPATH`                                             | adds paths to dependency and Iguana Python packages, if Python bindings are installed                                                     |

`this_iguana.sh` is compatible with `bash` and `zsh`, but not with `tcsh` or `csh`.
