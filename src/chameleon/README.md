# Chameleon

Given a `yaml` action function specification, `chameleon` generates:
- language bindings
- action function tests, for all languages

`chameleon` is run automatically by `meson` at build-time; all generated
code is produced in the build tree. If you want to test `chameleon`, you
may also run it locally:
```bash
chameleon --help
```
