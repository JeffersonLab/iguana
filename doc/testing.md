# Testing and Validating Algorithms

## Running a Single Test

The installed executable `iguana-test` is used for running single tests. Run
it with no arguments for a usage guide. For example:
```bash
iguana/bin/iguana-test validator -f data.hipo -n 0 -a clas12::MomentumCorrectionValidator -o validation_plots
```
will run the validator `MomentumCorrectionValidator` and write its output to `./validation_plots/`, assuming:
- your installation `prefix` is `./iguana`
- your data are found in `./data.hipo`

## Running All of the Tests

While in your build directory, you may run
```bash
meson test
```
to run all available tests. However, depending on your build options, not all of them may run. For example,
since the `validator` tests need input data, you need to set the build option `test_data_file` to a sample input
file, otherwise tests which need input data will not run.

For further usage of `meson test`, for example how to run a single test or control verbosity, run
```bash
meson test --help
```
> [!TIP]
> - if you are testing on a _large_ data set, you may need to increase the timeout with the `-t` option
>   and parallelize with the `-j` option
> - if a test is failing, use `--print-errorlogs` to see the `stderr` stream and add `--no-stdsplit` if you need to see the `stdout` stream;
>   if the streams' outputs are mixed up, try prepending `stdbuf` as:
>   ```bash
>   stdbuf -o0 meson test [OPTIONS]...
>   ```

> [!NOTE]
> All of the available tests run on the Continuous Integration (CI), so if you are developing and you submit a pull
> requests, you may rely on the CI logs to check if the tests were successful
