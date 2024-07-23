chameleon_sources = files(
  'chameleon',
  'src' / 'generator.rb',
  'src' / 'bind_c.rb',
)

ruby = find_program('ruby', version: '>=3.0.0', required: use_chameleon)
chameleon_gen = find_program(chameleon_sources[0], required: use_chameleon)
