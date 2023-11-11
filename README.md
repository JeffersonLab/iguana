# Iguana

Prototype design

## Dependencies
- `meson`
- `fmt`
  - Debian: `libfmt-dev`
  - Arch: `fmt`
- `hipo`
  - From: <https://github.com/gavalian/hipo>
  - To make `iguana` know where to find it, do one of:
    - use `install.rb` option `--hipo`
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
