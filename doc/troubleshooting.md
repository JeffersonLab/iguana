# Troubleshooting Notes

### 🔵 My output appears to be out of order: errors are not printed exactly when they occur

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

> [!NOTE]
> `stdbuf` on macOS may be installed as `gstdbuf`, from the Homebrew package `coreutils`.

### 🔵 I got a crash, but the stack trace (or debugger) is not telling me exactly where

Enable debugging symbols by setting the Iguana build option `buildtype` to `'debug'`, then rebuild.
Assuming you're in your build directory, run:
```bash
meson configure -Dbuildtype=debug
```
Then rebuild (`meson compile` and/or `meson install`).

Remember to revert this change and rebuild/re-install, so that Iguana runs with
full optimization when you are processing large data sets (`-Dbuildtype=release`).
