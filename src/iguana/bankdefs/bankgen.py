#!/usr/bin/env python3

"""
Reads an input JSON data model file, which defines the bank schema,
and generates a C++ source file setting the variable `iguana::BANK_DEFS`.
Doing this allows us to embed the bank schema definition in a library,
rather than having the library find the JSON file at runtime.
"""

import sys, json, textwrap

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

        out = open(output_file_name, 'w')
        out.write(textwrap.dedent(f'''\
        #include "iguana/algorithms/BankDefs.h"

        namespace iguana {{
          std::vector<BankDef> const BANK_DEFS = {{
        '''))

        i_bank_def = 0
        unique_item_ids = []
        for bank_def in bank_defs:

            i_bank_def += 1
            trail_bank_def = trailing_comma(bank_defs, i_bank_def)

            if(bank_def["item"] in unique_item_ids):
                print(f'ERROR: item ID {bank_def["item"]} is not unique in {input_file_name}', file=sys.stderr)
                exit(1)
            unique_item_ids.append(bank_def["item"])

            out.write(textwrap.indent(textwrap.dedent(f'''\
            {{
              .name    = "{bank_def["name"]}",
              .group   = {bank_def["group"]},
              .item    = {bank_def["item"]},
              .entries = {{
            '''), '    '))
            i_entry = 0
            for entry in bank_def['entries']:
                i_entry += 1
                trail_entry = trailing_comma(bank_def['entries'], i_entry)
                out.write(f'        {{ .name = "{entry["name"]}", .type = "{entry["type"]}" }}{trail_entry}\n')
            out.write(f'      }}\n')
            out.write(f'    }}{trail_bank_def}\n')

        out.write('  };\n')
        out.write('}\n')
        out.close()

    except json.decoder.JSONDecodeError:
        print(f'ERROR: failed to parse {input_file_name}; check its JSON syntax', file=sys.stderr)
        exit(1)

print(f'Generated {output_file_name}')
