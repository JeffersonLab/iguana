# Iguana

Prototype design

See [design notes](doc/design.md)

## Dependencies

### [`meson`](https://mesonbuild.com/)
Build system used by `iguana`
- likely available in your package manager
- you may also install `meson` with `pip`:
```bash
python -m pip install meson ninja
```

### [`fmt`](https://github.com/fmtlib/fmt)
C++ output formatting library
- likely available in your package manager, possibly under `libfmt`
- if you compile it yourself, include the `cmake` option `-DCMAKE_POSITION_INDEPENDENT_CODE=ON`

### [`hipo`](https://github.com/gavalian/hipo)
C++ HIPO API
- you will need to obtain and compile this yourself, or use a module on `ifarm`
- example `cmake` commands:
```bash
cmake -S /path/to/hipo_source_code -B build -DCMAKE_INSTALL_PREFIX=/path/to/hipo_installation
cmake --build build -j$(nproc)
cmake --install build
```

## Setup
First, configure your `iguana` build using `configure.py`:
```bash
configure.py --help
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

### Note for advanced users
If you are comfortable with `meson` and dependency resolution, there is no need to run `configure.py` or `install-iguana.sh`; you may instead run `meson` commands with your preferred options.

## Troubleshooting Notes

- if you redirect `stdout` and `stderr` to a file, you may notice that `stderr` lines are out-of-order with respect to the `stdout` lines; for example:
```bash
myAnalysisProgram                    # stdout and stderr print when they happen; ordering appears correct

myAnalysisProgram |& tee output.txt  # stderr prints when it happens, but stdout only prints when its buffer is full;
                                     # ordering appears mixed up
```
To redirect output to a file with the ordering preserved, run your executable with `stdout` unbuffered:
```bash
stdbuf -o0 myAnalysisProgram |& tee output.txt
```
