# Repository Maintenance

Notes for `iguana` maintainers

## Iguana version

Please remember to bump to the version number in [`meson.build`](/meson.build) prior to tagging a new release.

## Python Binding Dependencies

Keep the Python binding dependency versions reasonably up to date in the corresponding `requirements.txt` file:

- [`bind/python/requirements.txt`](/bind/python/requirements.txt)

## C++ Standard

We currently support up to the C++ standard defined in [`meson.build`](/meson.build); if you change this, you will also need to update:
- [ ] example build configurations in [`examples/build_with_*` subdirectories](/examples)
- [ ] in the [main CI workflow](/.github/workflows/ci.yml), the standard used for the ROOT build (`CMAKE_CXX_STANDARD`)
- [ ] any mention of the standard in documentation

## Code Ownership

We maintain a [`CODEOWNERS` file](/CODEOWNERS) to track who wrote and maintains each file. If you make significant contributions to any part of the code, please update `CODEOWNERS`. Pull requests that edit a file that you maintain will automatically request for your review, when marked as ready.

## Auto-Formatting

We provide a [`.clang-format`](/.clang-format) file for auto-formatting C++ code. If your system has `clang-format`,
then `meson` will create a `ninja` build target called `clang-format`, which you may run as
```bash
ninja -C <builddir> clang-format
```
We also use `meson.format` to format the `meson` build files.

For semi-automation, the script `.github/auto-format.sh` will create a new
`git` branch, and then run the auto-formatters; run `git diff` to see the
proposed changes.
