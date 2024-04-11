module iguana_clas12_EventBuilderFilter

  use, intrinsic :: iso_c_binding
  implicit none

  interface
    logical(c_bool) function action_filter(algo, pid) bind(C, name='iguana_clas12_EventBuilderFilter_Filter')
      import
      type(c_ptr), value, intent(in) :: algo
      integer(c_int), value, intent(in) :: pid
    end function
  end interface

end module
