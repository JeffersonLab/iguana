# Iguana

Prototype design

See [design notes](doc/design.md)

## Dependencies

Dependencies likely available in your package manager:
- [`meson`](https://mesonbuild.com/) version 1.1 or newer
- [`fmt`](https://github.com/fmtlib/fmt) version 10 or newer

Dependencies you may need to build yourself, unless available on a Jefferson Lab computer:
- [`hipo`](https://github.com/gavalian/hipo)
  - To make `iguana` know where to find it, do one of:
    - use option `--hipo` when running `install.rb`
    - symlink `./hipo` to the installation
    - set `$HIPO` to the installation
    - use `meson` build option `-Dhipo` (or its [default](meson.options))

## Setup
Run (from any directory):
```bash
install.rb --help        # print the usage guide
install.rb               # default: install to ./iguana
```
Alternatively, use `meson` for more control.

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
