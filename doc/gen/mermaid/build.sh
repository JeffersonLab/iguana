#!/usr/bin/bash
# requires `mermaid-cli`
# NOTE: this is not automated by `meson`, to avoid introducing `mermaid-cli` dependency;
# this is okay, since we do not expect this figure to change often
exec mmdc \
  -b transparent \
  -w 1600 \
  -i flowchart_usage.mmd \
  -o flowchart_usage.png
