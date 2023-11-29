# Iguana

Prototype design

See [design notes](doc/design.md)

## Dependencies

Dependencies likely available in your package manager:
- [`meson`](https://mesonbuild.com/)
- [`fmt`](https://github.com/fmtlib/fmt)

Dependencies you may need to build yourself, unless available on a Jefferson Lab computer:
- [`hipo`](https://github.com/gavalian/hipo)

## Setup
First, configure your `iguana` build using `configure.py`:
```bash
configure.py --help
```
The `--help` option will print the usage guide.
Unless the dependencies are installed in one of the system default locations, you will need to specify the path to each of them, _e.g._,
```bash
./configure.py --hipo /path/to/hipo/installation_prefix
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
