# Example Building with a Makefile

This example demonstrates how to build an executable that depends on `iguana`, using a `Makefile`

### Summary
You need to include the headers and libraries with the appropriate `-I`, `-L`, and
`-l` flags; see the example `Makefile`. For `iguana`, you can use `pkg-config` to get them:
```bash
export PKG_CONFIG_PATH=/path/to/iguana/installation
pkg-config --libs iguana
pkg-config --cflags iguana
```

### Details
- `Makefile`: the file used to build this example
- `test.sh`: demonstration of _how_ to build and run this example
  - this script is _used by the CI_, and is _not_ meant to be used locally
  - note that you will need to customize how dependencies are found
