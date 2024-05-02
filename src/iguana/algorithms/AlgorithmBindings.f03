module iguana_algorithm

  use, intrinsic :: iso_c_binding
  implicit none

contains

  subroutine iguana_algo_create(algo_idx, algo_name) bind(C)
    integer(c_int), intent(out) :: algo_idx
    character(c_char), intent(in) :: algo_name
  end subroutine

end module
