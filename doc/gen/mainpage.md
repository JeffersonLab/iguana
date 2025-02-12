# Iguana User's Guide
<img src="./logo.png" width=25%/>

This documentation shows how to use the Iguana algorithms. For more documentation, see the [**Documentation Front Page**](https://github.com/JeffersonLab/iguana/blob/main/README.md)

| Quick Links ||
| --- | --- |
| @spacer [List of Algorithms](#algo) @spacer | @spacer [Examples of Code](#secExample) @spacer |
| @spacer [List of Action Functions](#action) @spacer | @spacer [Configuring Algorithms](#secConfiguring) @spacer |

<br><hr>

<br>
@anchor secExample
## Example Analysis Code Using Iguana

To see Iguana algorithms used in the context of analysis code, with **various languages** and **use cases**, see:

| @spacer Examples @spacer ||
| --- | --- |
| @spacer [C++ Examples](#examples_cpp) @spacer         | for users of `ROOT`, `clas12root`, _etc_.                   |
| ^                                                     | includes guidance on how to build Iguana with your C++ code |
| @spacer [Python examples](#examples_python) @spacer   | for users of Python tools, such as `PyROOT`                 |
| @spacer [Fortran examples](#examples_fortran) @spacer | See also the [Fortran usage guide](#fortran_usage_guide)    |

The general way to use an Iguana algorithm is as follows; see examples for more details:

1. Decide [which algorithms](#algo) you want to use
    - Tip: you may use `iguana::AlgorithmSequence` to help run a _sequence_ of algorithms
2. Check each algorithm configuration, and [adjust it if you prefer](#secConfiguring)
3. Start each algorithm, which "locks in" its configuration:
    - call `Start(banklist)` if you use [**the HIPO API**](https://github.com/gavalian/hipo)
    - call `Start()` otherwise (_i.e._, if you use Action Functions)
4. In the event loop, run the algorithm:
    - call `Run(...)` if you use [**the HIPO API**](https://github.com/gavalian/hipo)
    - call the action functions otherwise
5. Proceed with your analysis
    - if you called `Run(...)`, the banks will be filtered, transformed, and/or created
    - if you called action functions, you will need to handle their output yourself
    - in either case, see [guidance on how to run algorithms](#secRunning) for more details
6. After your event loop, stop each algorithm by calling `Stop()`

Please let the maintainers know if your use case is not covered in any examples or if you need any help.

<br><hr>

<br>
@anchor secAlgorithms
## Algorithms

An Iguana algorithm is a function that maps input HIPO bank data to output data. There are a few different types of algorithms, based on how they act on HIPO data:

| Type | Description | Example |
| --- | --- | --- |
| **Filter** | Filters rows of a bank based on a Boolean condition | `iguana::clas12::FiducialFilter`: filter particles with fiducial cuts |
| **Transformer** | Transform (mutate) elements of a bank | `iguana::clas12::MomentumCorrection`: correct particle momenta |
| **Creator** | Create a new bank | `iguana::physics::InclusiveKinematics`: calculate inclusive kinematics @latex{x}, @latex{Q^2}, _etc_. |

The available algorithms are:

- [Algorithms organized by Namespace](#algo_namespaces)
- [Full List of Algorithms](#algo)

<br><hr>

<br>
@anchor secRunning
## How to Run Algorithms

Algorithms may be run using either:

- [Common Functions](#secCommon): for users of the [**the HIPO API**](https://github.com/gavalian/hipo), which is likely the case if you are using C++, _e.g._, via `clas12root`
- [Action Functions](#secAction): for all other users

The next sections describe each of these.

@important It is highly recommended to read an algorithm's documentation carefully before using it.

<br>
@anchor secCommon
### Common Functions

All algorithms have the following **Common Functions**, which may be used in analysis code that uses [**the HIPO API**](https://github.com/gavalian/hipo). These functions act on `hipo::bank` objects, and are designed to be called at certain points in an analysis of HIPO data:

| Common Functions ||
| --- | --- |
| `iguana::Algorithm::Start` | To be called before event processing |
| `iguana::Algorithm::Run` | To be called for every event |
| `iguana::Algorithm::Stop` | To be called after event processing |

The algorithms are implemented in C++ classes which inherit from the base class `iguana::Algorithm`; these three class methods are overridden in each algorithm.

The `iguana::Algorithm::Run` function should be called on every event; the general consequence depends on the
algorithm type:

<table>
<tr> <th>Type</th> <th>How to handle the results</th> </tr>
<tr> <td>**Filter**</td> <td>
The involved banks will be filtered.
To iterate over _filtered_ bank rows, use the function `hipo::bank::getRowList`,
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
</td> </tr>
<tr> <td>**Transformer**</td> <td>
The transformed `hipo::bank` will simply have
the relevant bank elements changed. For example, momentum correction algorithms
typically change the particle momentum components.
</td> </tr>
<tr> <td>**Creator**</td> <td>
Creator-type algorithms will simply create a new `hipo::bank` object, appending
it to the end of the input `hipo::banklist`. An initial version is created upon calling
`iguana::Algorithm::Start`, so that you may begin to reference it; it is helpful to
use `hipo::getBanklistIndex` (see [the examples for details](#secExample)).
</td> </tr>
</table>

Internally, `iguana::Algorithm::Run` calls Action Functions, which are described in the next section.

<br>
@anchor secAction
### Action Functions

The action functions do the _real_ work of the algorithm, and are meant to be
easily callable from _any_ analysis, even if HIPO banks are not directly used.
These functions are unique to each algorithm, so view the algorithm
documentation for details, or browse the full list:

- [List of all Action Functions](#action)

Action function parameters are supposed to be _simple_: numbers or lists of numbers, preferably obtainable _directly_
from HIPO bank rows. The return type of an action function depends on the algorithm type:

| Algorithm Type | Action Function Output |
| --- | --- |
| **Filter** | returns `bool` whether or not the filter passes |
| **Transformer** | returns the transformed parameters |
| **Creator** | returns a simple `struct` of parameters (corresponding to a created bank row) |

Some algorithms have action functions which require a number from _all_ of the rows of a bank; this distinction
motivates further classification of action functions:

| Action Function Type | Description |
| --- | --- |
| **Scalar** | All inputs and outputs are scalar quantities (single values). This type of function may be used on a single bank row. |
| **Vector** | All inputs and outputs are vector quantities (lists of values). This type of action function needs values from all of the bank rows. |
| **Mixed** | Inputs and outputs are scalar or vector quantities. |

To maximize compatibility with user analysis code, these functions are overloaded:
- for every scalar function, there should be an vector function which calls the scalar function iteratively
- not every vector function can have a corresponding scalar function, since some action functions need several bank rows' data

Finally, it is important to note _when_ to call action functions in the analysis code. For example, some action functions should be
called _once_ every event, while others should be called for every particle of the `REC::Particle` bank. Some algorithms
have _both_ of these types of functions, for example:
- `PrepareEvent`, which must be called first, at the beginning of an event's analysis
- `FilterParticle`, which must be called on every particle, using the _output_ of `PrepareEvent`

It is highly recommended to read an algorithm's documentation carefully before using it, _especially_ if you use action functions.

<br><hr>

<br>
@anchor secConfiguring
## How to Configure Algorithms

Most of the algorithms are configurable using a YAML configuration file. If you are using Iguana for your analysis, you likely want to customize the algorithm configurations.

The default configuration is installed in the `etc/` subdirectory of the Iguana installation. If you have set the Iguana environment variables using, _e.g._ `source this_iguana.sh`, or if you are using the version of Iguana installed on `ifarm`, you will have the environment variable `$IGUANA_CONFIG_PATH` set to include this `etc/` directory.

There are a few ways to configure the algorithms; see the sections below for the options

@important
While algorithm developers are encouraged _not_ to make breaking changes to their algorithms or configuration, in some cases certain changes cannot be prevented. Thus if you have your own algorithm configurations, you may want to keep up-to-date on any changes of the algorithm. We will try to announce all breaking changes in the Iguana release notes.

@note
If the Iguana installation is relocated, the environment variable `$IGUANA_CONFIG_PATH` _must_ be used at runtime.

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

