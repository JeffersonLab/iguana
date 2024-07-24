chameleon_sources = files(
  'chameleon',
  'src' / 'generator.rb',
  'src' / 'bind_c.rb',
)

ruby = find_program(
  'ruby',
  version: run_command(meson.project_source_root() / 'meson' / 'minimum-version.sh', 'ruby', check: true).stdout().strip(),
  required: use_chameleon,
)
chameleon_gen = find_program(chameleon_sources[0], required: use_chameleon)
