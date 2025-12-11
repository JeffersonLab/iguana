# Testing and Validating Algorithms

There are 2 ways to run tests:

- `iguana_test`: an installed executable used for running single tests; this offers more control at the expense of
  being less user friendly
- `meson test`: user-friendly automation of `iguana_test`

!!! important
    Both of these assume you are currently in your **build directory**.
    Iguana must first be **installed**, so the compiled algorithms can find certain dependent files.

!!! tip
    All of the available tests run automatically on the Continuous Integration
    (CI), so if you are developing and you submit a pull requests, you may rely
    on the CI logs to check if the tests were successful

## `iguana_test`

`iguana_test` is found both in the installation's `bin/` directory and in your build directory,
``` bash
src/iguana/tests/iguana_test   # assuming your build directory layout is 'mirror' (the default)
```
Run `iguana_test` for a usage guide:
``` bash
src/iguana/tests/iguana_test            # usage guide
src/iguana/tests/iguana_test validator  # usage of the 'validator' command
src/iguana/tests/iguana_test algorithm  # usage of the 'algorithm' command
```
For example,
``` bash
src/iguana/tests/iguana_test validator -f ../data.hipo -n 0 -a clas12::MomentumCorrectionValidator -o ../validator_output
```
will run the validator `MomentumCorrectionValidator` using data from
`../data.hipo` and write its output to `../validator_output/`.

## `meson test`

This section shows some useful `meson test` commands; for more guidance, run `meson test --help`.

### Running single tests

To list the available tests, run one of the following:
``` bash
meson test --list                    # list all available tests
meson test --list --suite validator  # list the 'validator' tests
meson test --list --suite algorithm  # list the 'algorithm' tests
```

- the `validator` suite runs the algorithm validators
- the `algorithm` suite just runs the algorithms

In the following example commands, let's run a test with the name
```
iguana:validator / validator-clas12-MyAlgorithm
```
To run this test (which may fail, see below):
``` bash
meson test validator-clas12-MyAlgorithm           # just run it
meson test validator-clas12-MyAlgorithm --verbose # run and show output (stdout and stderr)
```
If you have not supplied the required arguments, in particular the input data
file, the test will fail. For the `validator` and `algorithm` test suites,
`iguana_test validator` and `iguana_test algorithm` are the respective
underlying test executable commands; you may pass arguments to them using the
`--test-args` option; for example:
``` bash
meson test validator-clas12-MyAlgorithm --verbose --test-args '\-f ../my_hipo_file.hipo \-n 300 \-o ../validator_output'
```
See above for `iguana_test` usage guidance.

!!! note
    Note the usage of an escaped hyphen, `\-` instead of `-`; depending on your system, you may or may not need to use escaped hyphens in
    `--test-args` arguments, otherwise you will see errors such as `argument --test-args: expected one argument`.

Alternatively to using `--test-args`, you may set _default_ `iguana_test`
arguments using Iguana **build options**:

- The options that are relevant for tests are prefixed by `test_`
- Set options using `meson configure` and re-running `meson install`
  (see [the setup guide for guidance](setup.md))
- Using `--test-args` will override these build options.

### Running multiple tests

For multiple tests, it's strongly recommended to use **build options** rather than
`--test-args`. To run multiple tests, just omit the test name:
``` bash
meson test                        # run all of the tests
meson test --suite validator      # run all of the validator tests
meson test --suite validator -j4  # with 4 parallel processes
```

!!! tip
    if you are testing on a _large_ data set, you may need to increase the timeout with the `-t` option
    and parallelize with the `-j` option
