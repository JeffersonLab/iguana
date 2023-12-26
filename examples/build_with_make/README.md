# Example Building with a Makefile

This example [`Makefile`](Makefile) demonstrates how to build an executable that depends on `iguana`.

See [note on dependency resolution](../../doc/dependency_resolution.md) first.

You need to include the headers and libraries with the appropriate `-I`, `-L`,
and `-l` flags. For dependencies with a `pkg-config` file, such as `iguana`,
you can use `pkg-config` to get these flags:
```bash
pkg-config --libs iguana
pkg-config --cflags iguana
```
- These commands are used in the example `Makefile`
