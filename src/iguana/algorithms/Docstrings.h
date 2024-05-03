// unused header file for namespace docstrings

/// @defgroup algo_namespaces Algorithm Namespaces
///
/// The algorithms are organized into **namespaces**; click the links to view each of their algorithms,
/// or @ref algo "browse the full list".
///
/// @{

/// General, top-level namespace for algorithms and infrastructure. For algorithms and bindings, see its sub-namespaces.
namespace iguana {}

/// Example algorithms
namespace iguana::example {}

/// CLAS12 algorithms
namespace iguana::clas12 {}

/// Physics algorithms
namespace iguana::physics {}

/// @}

/////////////////////////////////////////////////////////////////////////////////

/// @defgroup binding_namespaces Fortran Bindings Namespaces
///
/// @brief C bindings, for Fortran usage
///
/// The functions in these namespaces are designed to provide bindings for Fortran (and for C).
/// The function names are all lowercase, and end in an underscore, to permit automatic binding
/// to Fortran 77. Visit each namespace page to view the available functions.
///
/// To use a function in Fortran, call it as a subroutine, but without the final
/// underscore; use `iso_c_binding` data types for the arguments, otherwise you
/// may have subtle runtime problems.
///
/// For example, consider the following C function:
/// ```cpp
/// void iguana_example_function_(int* a, float* b);
/// ```
/// To use this in Fortran:
/// ```fortran
/// use iso_c_binding
/// integer(c_int) a
/// real(c_float) b
/// call iguana_example_function(a, b)
/// ```
///
/// To use these bindings with your Fortran code, link against the installed `iguana` libraries and include the installed
/// Fortran module files. If you are on `ifarm`, use the following environment variables:
/// - `$IGUANA_FORTRAN_FLAGS`: to include the Module (`.mod`) files
/// - `$IGUANA_FORTRAN_LIBS`: for your linker
///
/// @see A Fortran example: `iguana-example-fortran.f`
///
/// @{

/// General `iguana` bindings
namespace iguana::bindings {}

/// CLAS12 algorithm action function bindings
namespace iguana::bindings::clas12 {}

/// Physics algorithm action function bindings
namespace iguana::bindings::physics {}

/// @}
