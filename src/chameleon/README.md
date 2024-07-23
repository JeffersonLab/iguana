# Chameleon

Chameleon is a code generator. Given a `yaml` action function specification,
a file named `Action.yaml` in an algorithm's source code directory, Chameleon
generates:

| Language | Bindings                 | Tests                    |
| ---      | ---                      | ---                      |
| Fortran  | :white_check_mark:       | :x:                      |
| Java     | :x:                      | :x:                      |
| Python   | :x:                      | :x:                      |

where
- :white_check_mark: means fully supported (as long as the algorithm has an `Action.yaml` file)
- :x: means this is not _yet_ supported

The main script is [`chameleon`](chameleon). For usage, run
```bash
chameleon --help
```
There is no need to run `chameleon` yourself unless you are developing or debugging it.
`chameleon` is run automatically by `meson` at build time and all generated
code is produced _within_ the _build directory_ as:
```
src/iguana/algorithms/chameleon_*
```
