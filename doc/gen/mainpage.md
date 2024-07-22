# Iguana User's Guide

This documentation shows how to use the Iguana algorithms.

For more documentation, see
- [**Documentation Front Page**](https://github.com/JeffersonLab/iguana/blob/main/README.md)

## Example Code in C++, Python, and Fortran

To see Iguana algorithms used in the context of analysis code, with **various languages** and **use cases**, see:
- \ref examples_frontpage "Examples of Analysis Code"

**NOTE:** If you're not familiar with Iguana, please read the sections below first.

## Algorithms

An Iguana algorithm is a function that maps input HIPO bank data to output data. The available algorithms are:

- \ref algo_namespaces "Algorithms organized by Namespace"
- \ref algo "Full List of Algorithms"

The algorithm types are defined based on what they do to HIPO bank data:

<table>
<tr><td> **Filter** </td><td> Remove rows of a bank based on some `bool` condition </td></tr>
<tr><td> **Transformer** </td><td> Transform (mutate) elements of a bank </td></tr>
<tr><td> **Creator** </td><td> Create a new bank </td></tr>
</table>

Most algorithms are configurable:
- [**Algorithm Configuration Guide**](https://github.com/JeffersonLab/iguana/blob/main/doc/configuration.md)

### Algorithm Common Functions: for HIPO C++ Users

All algorithms have the following functions, which may be used in analysis code
that uses [**the HIPO C++ API**](https://github.com/gavalian/hipo); for
analysis code that does not, skip to the **Action Functions** section.

<table>
<tr><td> `iguana::Algorithm::Start` </td><td> Run before event processing </td></tr>
<tr><td> `iguana::Algorithm::Run` </td><td> Run for every event </td></tr>
<tr><td> `iguana::Algorithm::Stop` </td><td> Run after event processing </td></tr>
</table>

### Algorithm Action Functions: for All Users

The action functions do the _real_ work of the algorithm, and are meant to be
easily callable from _any_ analysis, even if HIPO banks are not directly used.
These functions are unique to each algorithm, so view the algorithm
documentation for details, or browse the full list:

- \ref action "List of all Action Functions"

The action functions have types that correspond to the algorithm types.
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

