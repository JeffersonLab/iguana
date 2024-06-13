# Configuring the Algorithms

Most of the algorithms are configurable using a YAML configuration file. If you are using Iguana for your analysis, you likely want to customize the algorithm configurations.

The default configuration is installed in the `etc/` subdirectory of the Iguana installation. If you have set the Iguana environment variables using, _e.g._ `source this_iguana.sh`, or if you are using the version of Iguana installed on `ifarm`, you will have the environment variable `$IGUANA_CONFIG_PATH` set to include this `etc/` directory.

There are a few ways to configure the algorithms; see the sections below for the options

> [!IMPORTANT]
> While algorithm developers are encouraged _not_ to make breaking changes to their algorithms or configuration, in some cases certain changes cannot be prevented. Thus if you have your own algorithm configurations, you may want to keep up-to-date on any changes of the algorithm. We will try to announce all breaking changes in the Iguana release notes.

> [!NOTE]
> If the Iguana installation is relocated, the environment variable `$IGUANA_CONFIG_PATH` _must_ be used at runtime.

## Option 1: Copy the default directory, and modify

First, copy the default configuration directory to your work area, or to your analysis code source tree; we'll call the copied directory `my_iguana_config`, as an example. If `$IGUANA_CONFIG_PATH` is the default configuration directory (_i.e._ you have not set or modified this variable yourself), you may run:
```bash
cp -r $IGUANA_CONFIG_PATH my_iguana_config
```

You may then freely modify any configuration file within `my_iguana_config/`. To use this directory in your algorithms, you may do any one of the following:

1. Since `$IGUANA_CONFIG_PATH` allows multiple paths, delimited by colons (`:`), prepend `my_iguana_config/` to `$IGUANA_CONFIG_PATH`; the safest way is to use an absolute path:
```bash
export IGUANA_CONFIG_PATH=`pwd`/my_iguana_config:$IGUANA_CONFIG_PATH   # bash or zsh
setenv IGUANA_CONFIG_PATH `pwd`/my_iguana_config:$IGUANA_CONFIG_PATH   # tcsh or csh
```
The algorithms will then search `my_iguana_config` for the configuration before searching the default paths. You may add multiple paths, if needed.
Paths which appear first in `$IGUANA_CONFIG_PATH` will be prioritized when the algorithm searches for configuration parameters; this behavior is similar to that of `$PATH` or `$LD_LIBRARY_PATH`.

2. Use `Algorithm::SetConfigDirectory` instead of prepending `$IGUANA_CONFIG_PATH`.

## Option 2: Write your own YAML file

Make a new YAML file, containing just the options you want to override; use `Algorithm::SetConfigFile` to use this file for each algorithm.
See existing algorithms' configuration, and just copy what you need into your own YAML file; be mindful of the indentation, and that the top-level set of YAML nodes are the algorithm names. For example:

Algorithm default configuration files:
```yaml
### default configuration file 1
physics::AlgorithmA
  cuts: [-1, 1]
```
```yaml
### default configuration file 2
physics::AlgorithmB
  value: 3
```
Custom YAML file, widening `AlgorithmA`'s `cuts` and increasing `AlgorithmB`'s `value`:
```yaml
### custom YAML file
physics::AlgorithmA
  cuts: [-2, 2]

physics::AlgorithmB
  value: 5
```
