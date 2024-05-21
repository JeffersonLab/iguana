# Testing and Validating Algorithms

There are 2 ways to run tests:
- `iguana-test`: an installed executable used for running single tests
- `meson test`: automates the usage of `iguana-test`

Both of these assume you are currently in your **build directory**.
Iguana must first be **installed**, so the compiled algorithms can find certain dependent files.

## `iguana-test`

`iguana-test` is found both in the installation's `bin/` directory and in your build directory,
```bash
src/iguana/tests/iguana-test
```
Run `iguana-test` for a usage guide:
```bash
iguana-test
iguana-test validator # usage of the 'validator' command
```
For example,
```bash
iguana/bin/iguana-test validator -f data.hipo -n 0 -a clas12::MomentumCorrectionValidator -o validation_plots
```
will run the validator `MomentumCorrectionValidator` and write its output to `./validation_plots/`, assuming:
- your installation `prefix` is `./iguana`
- your data are found in `./data.hipo`

## `meson test`

Just run
```bash
meson test
```
to run all available tests. However, depending on your build options, not all of them may succeed. For example,
since the `validator` tests need input data, you need to set the build option `test_data_file` to a sample input
file, otherwise tests which need input data will fail. For such failing tests, you may use the `--test-args` option
to override the default ones used for the underlying `iguana-test`.

For further usage of `meson test`, see:
```bash
meson test --help
```
Here are some example useful `meson test` commands:
```bash
# listing
meson test --list                    # list all available tests
meson test --list --suite validator  # list the 'validator' tests
meson test --list --suite algorithm  # list the 'algorithm' tests

# run a single test, with the following name according to 'meson test --list':
#   iguana:validator / validator-clas12-MyAlgorithm
meson test validator-clas12-MyAlgorithm           # just run it
meson test validator-clas12-MyAlgorithm --verbose # show output (stdout and stderr)

# 'iguana-test validator' is under the hood; you may pass arguments to it with '--test-args':
meson test validator-clas12-MyAlgorithm --test-args '-f my_hipo_file.hipo -n 300'

# running multiple tests
meson test --suite validator      # run all of the validator tests
meson test --suite validator -j4  # with 4 parallel processes
```

> [!TIP]
> - if you are testing on a _large_ data set, you may need to increase the timeout with the `-t` option
>   and parallelize with the `-j` option

> [!NOTE]
> All of the available tests run on the Continuous Integration (CI), so if you are developing and you submit a pull
> requests, you may rely on the CI logs to check if the tests were successful
