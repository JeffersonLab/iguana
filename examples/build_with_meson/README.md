# Example Building with Meson

This example demonstrates how to build an executable that depends on `iguana`, using `meson`.

### Summary
Include `iguana` as a dependency in your `meson.build` file:
```meson
dependency('iguana')
```

### Details
- `meson.build`: the file used to build this example
- `test.sh`: demonstration of _how_ to build and run this example
  - this script is _used by the CI_, and is _not_ meant to be used locally
  - note the various build options used to specify dependency locations
