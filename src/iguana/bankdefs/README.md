# Iguana Bank Definitions

Creator algorithms create new banks; the files in this directory describe the new banks, in particular, [`iguana.json`](iguana.json).

This JSON file follows a similar format as the `coatjava` bank definitions and may be used as a reference; the only change is the addition of the `algorithm` key, which is used to inform which algorithm creates the bank. See [the user guide](https://jeffersonlab.github.io/iguana/doxygen) for more general information about the format of this JSON file.

## Additional notes for developers
- [`bankgen.py`](bankgen.py) will read the JSON file and generate C++ code to handle the bank creation
    - while you are developing your creator algorithm, you may first update the JSON file, then compile the code; this will run `bankgen.py`, which will generate `BankDefs.h` and `BankDefs.cc` within the _build_ directory, so you may inspect those files for further information if this documentation is not clear
    - compilation will fail if the JSON syntax is incorrect
- for action functions' convenience, `bankgen.py` will generate a C++ `struct` with the name `[ALGORITHM_NAME]Vars` (the algorithm name with `Vars` appended)
    - this `struct` may be used as the _return value_ type of action functions, if needed
    - the variables in the `struct` have the same name and types as those defined in the bank "entries"; the "info" docstrings are used by `doxygen`
- add a new bank to the end of the JSON file, and increment the "item" number
    - the "group" and "item" numbers must be unique for all banks (including those upstream in `coatjava`)
    - all Iguana banks must use "group" `30000`
- values of "info" entries must be `doxygen` docstrings
    - they are used in the documentation generation
    - all backslashes must be escaped, _i.e._ instead of `\` write `\\`, otherwise the JSON syntax will be invalid
- our convention is that the bank name matches the algorithm name, but that does not have to be followed (early creator algorithms did not have this convention)
