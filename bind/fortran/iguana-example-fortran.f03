program iguana_example_fortran

  use, intrinsic :: iso_c_binding
  implicit none

  ! iguana bindings interface
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

  ! action functions interface
  interface
    logical(c_bool) function action_filter(algo, pid) bind(C, name='iguana_clas12_EventBuilderFilter_Filter')
      import
      type(c_ptr), value, intent(in) :: algo
      integer(c_int), value, intent(in) :: pid
    end function
  end interface

  ! -----------------------------------------------

  integer                   :: argc, arglen
  character(:), allocatable :: arg, in_file
  integer(c_int)            :: num_events

  integer :: reader_status
  integer :: counter
  integer :: nrows
  integer :: nr

  integer(c_int)  :: pid(50)
  logical(c_bool) :: accept(50)

  integer :: i, j

  type(c_ptr) :: event_builder_filter

  ! -----------------------------------------------

  ! parse arguments
  num_events = 10
  argc = command_argument_count()
  do i = 0, argc
    if(allocated(arg)) then
      deallocate(arg)
    end if
    call get_command_argument(number=i, length=arglen)
    allocate(character(arglen) :: arg)
    call get_command_argument(number=i, value=arg)
    select case(i)
      case(0)
        if(argc.eq.0) then
          print *, 'USAGE: ' // arg // ' [HIPO data file] [number of events (0 for all)]'
          error stop 'please specify a HIPO file'
        end if
      case(1)
        allocate(character(arglen) :: in_file)
        in_file = arg
      case(2)
        read(arg,*) num_events
    end select
  end do

  ! create iguana algorithms
  event_builder_filter = iguana_algo_create('clas12::EventBuilderFilter')
  ! momentum_corrections = iguana_algo_create('clas12::MomentumCorrection') ! FIXME: can't create 2 algos

  ! open the HIPO file
  if(allocated(in_file)) then
    call hipo_file_open(in_file//c_null_char) ! be sure to terminate with null character
  else
    error stop 'in file name is not allocated'
  end if
  reader_status = 0
  counter       = 0

  ! start iguana algorithms
  call iguana_algo_start(event_builder_filter)

  ! event loop
  do while(reader_status.eq.0 .and. (num_events.eq.0 .or. counter.lt.num_events))

    ! read banks
    call hipo_file_next(reader_status)
    call hipo_read_bank('REC::Particle', nrows)
    call hipo_read_int('REC::Particle', 'pid', nr, pid, 50)

    ! call iguana filter
    do i=1, nrows
      accept(i) = action_filter(event_builder_filter, pid(i))
    end do

    ! print results
    write(*,*) '>>>>>>> event ', counter
    write(*,*) '        nrows ', nrows
    write(*,*) (pid(j), j=1, nrows)
    write(*,*) (accept(j), j=1, nrows)

    counter = counter + 1
  end do

  ! stop and destroy iguana algorithms
  call iguana_algo_stop_and_destroy(event_builder_filter)

end program
