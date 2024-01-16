# Example Algorithm Template

This directory includes a fully documented, simple example algorithm:
- [`ExampleAlgorithm.h`](ExampleAlgorithm.h)
- [`ExampleAlgorithm.cc`](ExampleAlgorithm.cc)

In this code, the following comment styles are used:
- `//#`: this is a comment which describes in detail what each line of the code does; you probably
  don't want this much detail in your own algorithm
- `///`: this is a docstring for [Doxygen](https://www.doxygen.nl/), for automated documentation generation;
  you'll need to use these when documenting your algorithm

To generate a template algorithm (without the `//#` comments) so you may get
started coding, run the `make_template.sh` script here:
```bash
# run with no arguments to see usage
src/iguana/algorithms/example/make_template.sh

# example: make a CLAS12 algorithm called AwesomeAlgorithm
src/iguana/algorithms/example/make_template.sh AwesomeAlgorithm clas12 src/iguana/algorithms/clas12
```

Once you have generated your new algorithm, add it to the appropriate
`meson.build` file (likely [`src/iguana/algorithms/meson.build`](../meson.build)),
and get started coding!

> [!TIP]
> Enable debugging symbols when building by setting the Iguana build option `buildtype` to `'debug'`.
