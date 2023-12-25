# Setup

## Install Python Packages
It's good practice to setup a virtual environment:
```bash
python -m venv .venv
source .venv/bin/activate
```

Then install packages with:
```bash
pip install -r bind/python/requirements.txt
```
- **Note**: If you get an error stating that `"Python.h"` cannot be found, you need to install Python development headers and static libraries; depending on your OS and package manager, the relevant package to install is something like `python3-dev` or `python3-devel`.
- **Note**: These bindings may not work for Python versions below 3.10

## Building the Python Bindings
Use the `--python` option when running `configure.py`, or edit your `build-iguana.ini` file
to set the `bind_python` option to `True`. Then, build and try the example:
```bash
./install-iguana.sh
source iguana/bin/this_iguana.sh
iguana/bin/iguana-example-bind.py
```
