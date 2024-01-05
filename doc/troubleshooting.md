# Troubleshooting Notes

### :large_blue_diamond: My output appears to be out of order: errors are not printed exactly when they occur

If you redirect `stdout` and `stderr` to a file, you may notice that `stderr` lines are out-of-order with respect to the `stdout` lines; for example:
```bash
myAnalysisProgram                    # stdout and stderr print when they happen; ordering appears correct

myAnalysisProgram |& tee output.txt  # stderr prints when it happens, but stdout only prints when its buffer is full;
                                     # ordering appears mixed up
```
To redirect output to a file with the ordering preserved, run your executable with `stdout` unbuffered:
```bash
stdbuf -o0 myAnalysisProgram |& tee output.txt
```

### :large_blue_diamond: I got a crash, but the stack trace (or debugger) is not telling me exactly where

Try enabling debugging symbols, either by:
- set built-in option `buildtype` to `'debug'` in your build-configuration `.ini` file (or in your `meson` command)
- use `--debug` when running `configure.py`

Then re-build `iguana`.

Remember to revert this change and re-build, so that `iguana` runs with full optimization when you are processing large data sets (`buildtype = 'release'`).
