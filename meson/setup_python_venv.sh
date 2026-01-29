#!/usr/bin/env bash
set -euo pipefail
python -m venv .venv
source .venv/bin/activate
python -m pip install -r bind/python/requirements.txt
python -m pip install -r doc/mkdocs/requirements.txt
echo "DONE: now source one of the activate scripts in '.venv/bin/'"
