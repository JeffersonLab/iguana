# Iguana User's Guide
<img src="./logo.png" width=25%/>

This documentation shows how to use the Iguana algorithms. For more documentation, see the [**Documentation Front Page**](https://github.com/JeffersonLab/iguana/blob/main/README.md)

**Quick Links:**
<table>
    <tr>
        <td>@spacer[List of Algorithms](#algo)@spacer</td>
        <td>@spacer[Examples of Code](#secExample)@spacer</td>
    </tr>
    <tr>
        <td>@spacer[List of Action Functions](#action)@spacer</td>
        <td>@spacer[Configuring Algorithms](#secConfiguring)@spacer</td>
    </tr>
</table>

<br /><hr /><br />

@section secExample Example Analysis Code Using Iguana

To see Iguana algorithms used in the context of analysis code, with **various languages** and **use cases**, see:
<table>
    <tr>
        <td>@spacer[C++ Examples](#examples_cpp)@spacer</td>
        <td>
            <ul>
                <li>for users of `ROOT`, `clas12root`, _etc_.</li>
                <li>includes guidance on how to build Iguana with your C++ code</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>@spacer[Python examples](#examples_python)@spacer</td>
        <td>
            <ul>
                <li>for users of Python tools, such as `PyROOT`</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>@spacer[Fortran examples](#examples_fortran)@spacer</td>
        <td>
            <ul>
                <li>See also the [Fortran usage guide](#fortran_usage_guide)</li>
            </ul>
        </td>
    </tr>
</table>

The general way to use an Iguana algorithm is as follows; see examples for more details:

1. Decide [which algorithms](#algo) you want to use
    - Tip: you may use `iguana::AlgorithmSequence` to help run a _sequence_ of algorithms
2. Check each algorithm configuration, and [adjust it if you prefer](#secConfiguring)
3. Start each algorithm, which "locks in" its configuration:
    - call `Start(banklist)` if you use [**the HIPO C++ API**](https://github.com/gavalian/hipo)
    - call `Start()` otherwise (_i.e._, if you use Action Functions)
4. In the event loop, run the algorithm:
    - call `Run(...)` if you use [**the HIPO C++ API**](https://github.com/gavalian/hipo)
    - call the action functions otherwise
5. Proceed with your analysis
    - if you called `Run(...)`, the banks will be filtered, transformed, and/or created
    - if you called action functions, you will need to handle their output yourself
    - in either case, see [guidance on how to run algorithms](#secRunning) for more details
6. After your event loop, stop each algorithm by calling `Stop()`

Please let the maintainers know if your use case is not covered in any examples or if you need any help.

<br /><hr /><br />

@section secAlgorithms Algorithms

An Iguana algorithm is a function that maps input HIPO bank data to output data. There are a few different types of algorithms, based on how they act on HIPO data:

<table>
    <tr>
        <th>Type</th>
        <th>Description</th>
        <th>Example</th>
    </tr>
    <tr>
        <td>**Filter**</td>
        <td>Filters rows of a bank based on a Boolean condition</td>
        <td>`iguana::clas12::FiducialFilter`: filter particles with fiducial cuts</td>
    </tr>
    <tr>
        <td>**Transformer**</td>
        <td>Transform (mutate) elements of a bank</td>
        <td>`iguana::clas12::MomentumCorrection`: correct particle momenta</td>
    </tr>
    <tr>
        <td>**Creator**</td>
        <td>Create a new bank</td>
        <td>`iguana::physics::InclusiveKinematics`: calculate inclusive kinematics @latex{x}, @latex{Q^2}, _etc_.</td>
    </tr>
</table>

The available algorithms are:

- [Algorithms organized by Namespace](#algo_namespaces)
- [Full List of Algorithms](#algo)

<br /><hr /><br />

@section secRunning How to Run Algorithms

All algorithms have the following functions, which may be used in analysis code that uses [**the HIPO C++ API**](https://github.com/gavalian/hipo); these functions act on `hipo::bank` objects.

- Note: If you do not use this API with `hipo::bank` objects, skip to the next section on [Algorithm Action Functions](#secActionFunctions).

<table>
<tr><td> `iguana::Algorithm::Start` </td><td> Run before event processing </td></tr>
<tr><td> `iguana::Algorithm::Run` </td><td> Run for every event </td></tr>
<tr><td> `iguana::Algorithm::Stop` </td><td> Run after event processing </td></tr>
</table>

<br />
### What to do after calling `iguana::Algorithm::Run`

The `Run` function is called on every event; the general consequence depends on the
algorithm type.

<table>
    <tr>
        <th>Type</th>
        <th>How to handle the results</th>
    </tr>
    <tr>
        <td>**Filter**</td>
        <td>
To iterate over _filtered_ bank rows, you _must_ use the function `hipo::bank::getRowList`,
rather than iterating from `0` up to `hipo::bank::getRows()`; for example, given a
bank object named `particle_bank`:
```cpp
for(auto const& row : particle_bank.getRowList()) {
    // loops over only the rows which pass the filter
}
```
```cpp
for(int row = 0; row < particle_bank.getRows(); row++) {
    // loops over ALL rows, regardless of whether they pass the filter
}
```
        </td>
    </tr>
    <tr>
        <td>**Transformer**</td>
        <td>
The transformed `hipo::bank` will simply have
the relevant bank elements changed. For example, momentum correction algorithms
typically change the particle momentum components.
        </td>
    </tr>
    <tr>
        <td>**Creator**</td>
        <td>
Creator-type algorithms will simply create a new `hipo::bank` object, appending
it to the end of the input `hipo::banklist`. An initial version is created upon calling
`iguana::Algorithm::Start`, so that you may begin to reference it; it is helpful to
use `hipo::getBanklistIndex` (see [the examples for details](#secExample)).
        </td>
    </tr>
</table>

<br />
@anchor secActionFunctions
### Algorithm Action Functions

The action functions do the _real_ work of the algorithm, and are meant to be
easily callable from _any_ analysis, even if HIPO banks are not directly used.
These functions are unique to each algorithm, so view the algorithm
documentation for details, or browse the full list:

- [List of all Action Functions](#action)

The action functions have types that correspond to the algorithm types. Filters,
for example, return `bool` results indicating whether the filter passed or not.
Furthermore, some action functions can only be useful if all of the rows of a
bank are included in its inputs, which motivates the following additional
classification:
<table>
<tr><td> **Scalar** </td><td>
All inputs and outputs are scalar quantities (single values).
This type of function may be used on a single bank row.
</td></tr>
<tr><td> **Vector** </td><td>
All inputs and outputs are vector quantities (lists of values).
This type of action function needs values from all of the bank rows.
</td></tr>
<tr><td> **Mixed** </td><td>
Inputs and outputs are scalar or vector quantities.
</td></tr>
</table>
To maximize compatibility with user analysis code, these functions are
overloaded, _e.g._, for every scalar function there is a corresponding vector
function that calls it on each element of its input vectors.

> [!IMPORTANT]
> Some algorithms have different types of action functions which are meant to be run in different loops.
> For example, an algorithm may require calling a `PrepareEvent` function on an event _before_ calling
> a particle-filtering action function on every particle. Read the algorithm's documentation carefully
> before using action functions.

<br /><hr /><br />

@section secConfiguring How to Configure Algorithms

Most of the algorithms are configurable using a YAML configuration file. If you are using Iguana for your analysis, you likely want to customize the algorithm configurations.

The default configuration is installed in the `etc/` subdirectory of the Iguana installation. If you have set the Iguana environment variables using, _e.g._ `source this_iguana.sh`, or if you are using the version of Iguana installed on `ifarm`, you will have the environment variable `$IGUANA_CONFIG_PATH` set to include this `etc/` directory.

There are a few ways to configure the algorithms; see the sections below for the options

> [!IMPORTANT]
> While algorithm developers are encouraged _not_ to make breaking changes to their algorithms or configuration, in some cases certain changes cannot be prevented. Thus if you have your own algorithm configurations, you may want to keep up-to-date on any changes of the algorithm. We will try to announce all breaking changes in the Iguana release notes.

> [!NOTE]
> If the Iguana installation is relocated, the environment variable `$IGUANA_CONFIG_PATH` _must_ be used at runtime.

### Option 1: Copy the default directory, and modify

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

### Option 2: Write your own YAML file

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

