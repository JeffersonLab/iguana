bankdef_json = files('iguana.json')

prog_bankgen_sources = files('bankgen.py')
prog_bankgen = find_program(prog_bankgen_sources)

# generate BankDefs.cpp from the JSON data model file
bankdef_tgt = custom_target(
  'bankdefs',
  input: [
    bankdef_json,
    prog_bankgen_sources,
    meson.project_source_root() / 'src' / 'iguana' / 'algorithms' / 'BankDefs.h',
  ],
  output: [ 'BankDefs.cpp' ],
  command: [ prog_bankgen, '@INPUT0@', '@OUTPUT0@' ],
)

# install the JSON data model file; iguana won't need it, but it can be useful for user reference
install_data(bankdef_json, install_dir: project_etc / 'bankdefs' / 'hipo4')
