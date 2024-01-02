# Repository Maintenance

Notes for `iguana` maintainers

## Iguana version

The `iguana` version is dynamically determined from the most recent `git` tag in the commit graph:

- [`meson/detect-version.sh`](../meson/detect-version.sh)

This will _only_ work if the source tree includes `.git/`, with tags and history including the most recent tag.
If the source code was obtained from a release tarball from GitHub, the `.git/` directory will be absent and version detection will fail.

Because of the fragility of version detection, we try to keep at least the _major_ version correct in the fallback version number of the version detection implementation.

## Python Binding Dependencies

Keep the Python binding dependency versions reasonably up to date in the corresponding `requirements.txt` file:

- [`bind/python/requirements.txt`](../bind/python/requirements.txt)

## C++ Standard

We currently support up to the C++ standard defined in:
- [`meson.build`](../meson.build)
- example build configurations in [`examples/build_with_*` subdirectories](../examples)
