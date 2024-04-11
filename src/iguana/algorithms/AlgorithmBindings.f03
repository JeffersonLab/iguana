module iguana_Algorithm

  use, intrinsic :: iso_c_binding
  implicit none

  interface

    type(c_ptr) function iguana_algo_create(algo_name) bind(C)
      import
      character(c_char), intent(in) :: algo_name
    end function

    subroutine iguana_algo_start(algo) bind(C)
      import
      type(c_ptr), value, intent(in) :: algo
    end subroutine

    subroutine iguana_algo_stop(algo) bind(C)
      import
      type(c_ptr), value, intent(in) :: algo
    end subroutine

    subroutine iguana_algo_destroy(algo) bind(C)
      import
      type(c_ptr), value, intent(in) :: algo
    end subroutine

    subroutine iguana_algo_stop_and_destroy(algo) bind(C)
      import
      type(c_ptr), value, intent(in) :: algo
    end subroutine

  end interface

end module
