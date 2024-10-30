#!/usr/bin/env python3

import sys
import json

if(len(sys.argv) < 3):
    print(f'USAGE {__file__} [INPUT_JSON] [OUTPUT]')
    exit(2)
input_file_name = sys.argv[1]
output_file_name = sys.argv[2]

def trailing_comma(arr, idx):
    if(idx < len(arr)):
       return ','
    else:
       return ''

with open(input_file_name) as input_file:

    try:
        bank_defs = json.load(input_file)

        print('  std::vector<BankDef> const bank_defs{')
        i_bank_def = 0
        for bank_def in bank_defs:
            i_bank_def += 1
            trail_bank_def = trailing_comma(bank_defs, i_bank_def)
            print(f'    {{')
            print(f'      .name = "{bank_def["name"]}",')
            print(f'      .group = {bank_def["group"]},')
            print(f'      .item = {bank_def["item"]},')
            print(f'      .entries = {{')
            i_entry = 0
            for entry in bank_def['entries']:
                i_entry += 1
                trail_entry = trailing_comma(bank_def['entries'], i_entry)
                print(f'        {{ .name = "{entry["name"]}", .type = "{entry["type"]}" }}{trail_entry}')
            print(f'      }}')
            print(f'    }}{trail_bank_def}')
        print('  };')

    except json.decoder.JSONDecodeError:
        print(f'ERROR: failed to parse {input_file_name}; check its JSON syntax', file=sys.stderr)
        exit(1)
