# Setup

## Install Dependency Packages

> [!TIP]
> Since you will need to install some additional Python packages, we recommend setting up a Python Virtual Environment, _e.g._, with `venv`.

Install the required packages with:
```bash
pip install -r bind/python/requirements.txt
```

> [!NOTE]
> If you get an error stating that `"Python.h"` cannot be found, you need to install Python development headers and static libraries; depending on your OS and package manager, the relevant package to install is something like `python3-dev` or `python3-devel`.

> [!NOTE]
> These bindings may not work for Python versions below 3.10

## Building the Python Bindings
To enable Python bindings when building `iguana`, set the build option `bind_python` to `True`
(rebuild `iguana` if you changed this option).

For Python to be able to find and use these bindings, you need to set some environment variables:
- `PYTHONPATH` must include the path to the `python` subdirectory of the `iguana` installation
- on macOS only, shared libraries must be found in `DYLD_LIBRARY_PATH`

> [!TIP]
> You may use `this_iguana.sh` to set these variables automatically. Assuming you installed Iguana to `./iguana/`:
> - on Linux: `source iguana/bin/this_iguana.sh`
> - on macOS: `source iguana/bin/this_iguana.sh ld` (where `ld` is needed to set `DYLD_LIBRARY_PATH`)

## Running the Examples

Example Python scripts are found in this directory as `iguana-example-*.py`; they will be installed in the `bin/` subdirectory.

Most of them are analogous to the C++ examples, but some may be specific to the Python bindings.
