bankdef_json = files('iguana.json')

prog_bankdef_gen_sources = files('bank_def_gen.py')
prog_bankdef_gen = find_program(prog_bankdef_gen_sources)

# generate BankDefs.cpp from the JSON data model file
bankdef_tgt = custom_target(
  'bankdefs',
  input: [
    bankdef_json,
    prog_bankdef_gen_sources,
    meson.project_source_root() / 'src' / 'iguana' / 'algorithms' / 'BankDefs.h',
  ],
  output: [ 'BankDefs.cpp' ],
  command: [ prog_bankdef_gen, '@INPUT0@', '@OUTPUT0@' ],
)

# install the JSON data model file; iguana won't need it, but it can be useful for user reference
install_data(bankdef_json, install_dir: project_etc / 'bankdefs' / 'hipo4')
