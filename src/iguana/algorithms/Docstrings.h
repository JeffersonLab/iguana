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
/// c     use the ISO_C_BINDING module:
///       use iso_c_binding
/// c     function arguments must be ISO_C_BINDING kinds:
///       integer(c_int) a
///       real(c_float) b
/// c     example function call:
///       call iguana_example_function(a, b)
/// ```
///
/// If the C function has a string parameter (`char const*`), make sure that the string that you pass
/// has the correct termination (`c_null_char`), even if it's a string literal; for example, the C function
/// ```cpp
/// void iguana_example_function_(char const* str);
/// ```
/// must be called in Fortran like this:
/// ```fortran
/// c     string variables need to be BOTH trimmed AND terminated:
///       character*1024 str
///       str = 'sample_string'
///       call iguana_example_function(trim(str)//c_null_char)
///
/// c     string literals just need to be terminated:
///       call iguana_example_function('sample_string'//c_null_char)
/// ```
///
/// To use these bindings with your Fortran code, link against the installed `iguana` libraries;
/// for example, if you use a `Makefile`, you may set the linking arguments to `$(IGUANA_LIBS)`,
/// ```make
/// IGUANA_LIBS = $(shell pkg-config iguana --libs --maximum-traverse-depth 2)
/// ```
/// then use `$(IGUANA_LIBS)` in your compilation calls. If you need to link Iguana dependency libraries
/// too, remove the `--maximum-traverse-depth 2` argument.
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
