# Repository Maintenance

Notes for `iguana` maintainers

## Iguana version

Make sure the version number in the top-level `meson.build` is correct, prior to tagging a new release.

## Python Binding Dependencies

Keep the Python binding dependency versions reasonably up to date in the corresponding `requirements.txt` file:

- [`bind/python/requirements.txt`](https://github.com/JeffersonLab/iguana/blob/main/bind/python/requirements.txt)

## C++ Standard

We currently support up to the C++ standard defined in the top-level `meson.build`; if you change this, you will also need to update:

- example build configurations in `examples/build_with_*` subdirectories
- any mention of the standard in documentation

## Code Ownership

We maintain a `CODEOWNERS` file to track who wrote and maintains each file. If you make significant contributions to any part of the code, please update `CODEOWNERS`. Pull requests that edit a file that you maintain will automatically request for your review, when marked as ready.

## Auto-Formatting

We provide a `.clang-format` file for auto-formatting C++ code. If your system has `clang-format`,
then `meson` will create a `ninja` build target called `clang-format`, which you may run as
``` bash
ninja -C <builddir> clang-format
```
We also use `meson.format` to format the `meson` build files.

For semi-automation, the script `.github/auto-format.sh` will create a new
`git` branch, and then run the auto-formatters; run `git diff` to see the
proposed changes.
