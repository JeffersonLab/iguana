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

## Building the Python Bindings
Use the `--python` option when running `configure.py`, or edit your `build-iguana.ini` file
to set the `bind_python` option to `True`. Then, build and try the example:
```bash
./install-iguana.sh
source iguana/bin/this_iguana.sh
iguana/bin/iguana-example-bind.py
```
