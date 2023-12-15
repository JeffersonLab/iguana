# Setup
```
python -m venv .python
source .python/bin/activate
python -m pip install cppyy
export HIPO=$(realpath ../install)         # path to hipo prefix
export LD_LIBRARY_PATH=$(pwd)/iguana/lib:$HIPO/lib
export PYTHONPATH=$(pwd)/iguana/python
```
- build with `bind_python` enabled
- run the installed `bin/iguana-python-example.py`
