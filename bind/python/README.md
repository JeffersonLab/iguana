# Setup

## Install Python Packages

> [!TIP]
> Since you will need to install some additional Python packages, we recommend setting up a Python Virtual Environment, _e.g._, with `venv`.

Install packages with:
```bash
pip install -r bind/python/requirements.txt
```

> [!NOTE]
> If you get an error stating that `"Python.h"` cannot be found, you need to install Python development headers and static libraries; depending on your OS and package manager, the relevant package to install is something like `python3-dev` or `python3-devel`.

> [!NOTE]
> These bindings may not work for Python versions below 3.10

## Building the Python Bindings
Use the `--python` option when running `configure.py`, or edit your `build-iguana.ini` file
to set the `bind_python` option to `True`. Then, build and try the example:
```bash
./install-iguana.sh
source iguana/bin/this_iguana.sh
iguana/bin/iguana-example-bind.py
```
