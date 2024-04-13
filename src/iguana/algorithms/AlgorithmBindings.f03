!> `iguana::Algorithm` Fortran bindings module
module iguana_Algorithm

  use, intrinsic :: iso_c_binding
  implicit none

  interface

    type(c_ptr) function iguana_algo_create(algo_name) bind(C)
      !> @see `iguana::bindings::iguana_algo_create`
      !> @param algo_name the name of the algorithm
      !> @returns a pointer to the algorithm
      import
      character(c_char), intent(in) :: algo_name
    end function

    subroutine iguana_algo_start(algo) bind(C)
      !> @see `iguana::bindings::iguana_algo_start`
      !> @param algo the algorithm
      import
      type(c_ptr), value, intent(in) :: algo
    end subroutine

    subroutine iguana_algo_stop(algo) bind(C)
      !> @see `iguana::bindings::iguana_algo_stop`
      !> @param algo the algorithm
      import
      type(c_ptr), value, intent(in) :: algo
    end subroutine

    subroutine iguana_algo_destroy(algo) bind(C)
      !> @see `iguana::bindings::iguana_algo_destroy`
      !> @param algo the algorithm
      import
      type(c_ptr), value, intent(in) :: algo
    end subroutine

    subroutine iguana_algo_stop_and_destroy(algo) bind(C)
      !> @see `iguana::bindings::iguana_algo_stop_and_destroy`
      !> @param algo the algorithm
      import
      type(c_ptr), value, intent(in) :: algo
    end subroutine

  end interface

end module
