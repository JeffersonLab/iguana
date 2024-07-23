# Chameleon

Chameleon is a code generator. Given a `yaml` action function specification,
a file named `Action.yaml` in an algorithm's source code directory, Chameleon
generates:

| Language | Bindings                 | Tests                    |
| ---      | ---                      | ---                      |
| Fortran  | :heavy_check_mark:       | :heavy_multiplication_x: |
| Java     | :heavy_multiplication_x: | :heavy_multiplication_x: |
| Python   | :heavy_multiplication_x: | :heavy_multiplication_x: |

where
- :heavy_check_mark: means fully supported (as long as the algorithm has an `Action.yaml` file)
- :heavy_multiplication_x: means this is not _yet_ supported

The main script is [`chameleon`](chameleon). For usage, run
```bash
chameleon --help
```
There is no need to run `chameleon` yourself unless you are developing or debugging it.
`chameleon` is run automatically by `meson` at build time and all generated
code is produced in the build tree at
```
src/iguana/algorithms/chameleon_*
```
