# Repository Maintenance

Notes for `iguana` maintainers

## Iguana version

The `iguana` version is dynamically determined from the most recent `git` tag in the commit graph:

- [`meson/detect-version.sh`](/meson/detect-version.sh)

This will _only_ work if the source tree includes `.git/`, with tags and history including the most recent tag.
If the source code was obtained from a release tarball from GitHub, the `.git/` directory will be absent and version detection will fail.

Because of the fragility of version detection, we try to keep at least the _major_ version correct in the fallback version number of the version detection implementation.

## Python Binding Dependencies

Keep the Python binding dependency versions reasonably up to date in the corresponding `requirements.txt` file:

- [`bind/python/requirements.txt`](/bind/python/requirements.txt)

## C++ Standard

We currently support up to the C++ standard defined in:
- [`meson.build`](/meson.build)
- example build configurations in [`examples/build_with_*` subdirectories](/examples)

## Code Ownership

We maintain a [`CODEOWNERS` file](/CODEOWNERS) to track who wrote and maintains each file. If you make significant contributions to any part of the code, please update `CODEOWNERS`. Pull requests that edit a file that you maintain will automatically request for your review, when marked as ready.

## Auto-Formatting

### C++

We provide a [`.clang-format`](/.clang-format) file for auto-formatting C++ code. If your system has `clang-format`,
then `meson` will create a `ninja` build target called `clang-format`, which you may run as
```bash
ninja -C <builddir> clang-format
```
For semi-automation, the script `.github/auto-format.sh` will create a new `git` branch, run `clang-format`, then commit and push it to the remote `origin`; the user may then open a pull request to apply the changes. This chore is useful to do before creating a new release.
