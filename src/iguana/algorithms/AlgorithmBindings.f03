module iguana_Algorithm
  !! Fortran bindings module for Iguana algorithms: use these functions and subroutines to
  !! create and interact with Iguana algorithm objects.
  !!
  !! Since your Fortran program takes ownership of algorithm objects, these functions and
  !! subroutines require you to pass algorithm object pointers (`type(c_ptr)`) as arguments.

  use, intrinsic :: iso_c_binding
  implicit none

contains

  type(c_ptr) function iguana_algo_create(algo_name) bind(C)
    !! Create an Iguana algorithm.
    character(c_char), intent(in) :: algo_name !! the name of the algorithm
  end function

  subroutine iguana_algo_start(algo) bind(C)
    !! Start an algorithm by calling `Algorithm::Start`
    type(c_ptr), value, intent(in) :: algo !! the algorithm
  end subroutine

  subroutine iguana_algo_stop(algo) bind(C)
    !! Stop an algorithm by calling `Algorithm::Stop`
    type(c_ptr), value, intent(in) :: algo !! the algorithm
  end subroutine

  subroutine iguana_algo_destroy(algo) bind(C)
    !! Destroy an algorithm by calling its destructor
    type(c_ptr), value, intent(in) :: algo !! the algorithm
  end subroutine

  subroutine iguana_algo_stop_and_destroy(algo) bind(C)
    !! Call [[iguana_algo_stop]] then [[iguana_algo_destroy]]
    type(c_ptr), value, intent(in) :: algo !! the algorithm
  end subroutine

end module
