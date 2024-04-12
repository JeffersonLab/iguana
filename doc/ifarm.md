# Building for `ifarm` or `cvmfs`

## Resolve dependencies

Follow [the dependency resolution guide](/doc/dependency_resolution.md) or run the dependency resolution script
```bash
meson/resolve-dependencies.py --help  # prints the usage guide
```

> [!TIP]
> Use `--ini` to generate an INI file, which may be combined with the INI file from the next step

## Build

Replace anything in `<angle brackets>` with the appropriate string; you may run these commands from any directory,
preferably outside the source tree. The set of preferred options for `ifarm` builds is contained in [`ifarm.ini`](/meson/native-files/ifarm.ini), which may be passed to `meson` as a "native file".

```bash
meson setup <build_directory> <source_directory> <build_options_from_resolve_dependencies> --native-file=<source_directory>/meson/native-files/ifarm.ini
meson configure <build_directory> --prefix=<absolute_path_to_installation_destination>
meson install -C <build_directory>
```

> [!TIP]
> You may also add your prefix to the INI file, instead of calling `meson configure --prefix`
