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
