module iguana_clas12_EventBuilderFilter
  !! `iguana::clas12::EventBuilderFilter` Fortran bindings module

  use, intrinsic :: iso_c_binding
  implicit none

contains
  logical(c_bool) function iguana_clas12_EventBuilderFilter_Filter(algo, pid) &
      bind(C, name='iguana_clas12_EventBuilderFilter_Filter')
    !! binds [action function](https://jeffersonlab.github.io/iguana/doxygen/action.html) `iguana::clas12::EventBuilderFilter::Filter`
    type(c_ptr), value, intent(in) :: algo !! the algorithm
    integer(c_int), value, intent(in) :: pid !! the particle PDG to check
  end function

end module
