#!/usr/bin/bash
# requires `mermaid-cli`
# NOTE: this is not automated by `meson`, to avoid introducing `mermaid-cli` dependency;
# this is okay, since we do not expect this figure to change often
exec mmdc \
  --backgroundColor transparent \
  --width 1600 \
  --input flowchart_usage.mmd \
  --output flowchart_usage.png
