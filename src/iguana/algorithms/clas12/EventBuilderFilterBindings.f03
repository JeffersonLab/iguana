!> `iguana::clas12::EventBuilderFilter` Fortran bindings module
module iguana_clas12_EventBuilderFilter

  use, intrinsic :: iso_c_binding
  implicit none

  interface
    !> @see `iguana::clas12::EventBuilderFilter::Filter`
    !> @param algo the algorithm
    !> @param pid see `iguana::clas12::EventBuilderFilter::Filter`
    !> @returns see `iguana::clas12::EventBuilderFilter::Filter`
    logical(c_bool) function action_filter(algo, pid) bind(C, name='iguana_clas12_EventBuilderFilter_Filter')
      import
      type(c_ptr), value, intent(in) :: algo
      integer(c_int), value, intent(in) :: pid
    end function
  end interface

end module
