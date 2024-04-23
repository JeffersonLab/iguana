c Fortran example demonstrating how to read a HIPO file and use
c its data with Iguana algorithms
c
c **Usage:**
c ```text
c iguana-example-fortran [HIPO_FILE] [NUM_EVENTS]
c
c   HIPO_FILE   the HIPO file to analyze
c
c   NUM_EVENTS  the number of events to analyze;
c               set to 0 to analyze all events
c ```

      program iguana_example_fortran
      implicit none

c     ------------------------------------------------------------------

      integer*4      argc
      character*1024 in_file ! HIPO file
      character*32   num_events_arg
      integer        num_events / 10 / ! num. events to read (0 = all)

      integer reader_status
      integer counter
      integer nrows
      integer nr

      integer MAX_NUM_ROWS
      parameter (MAX_NUM_ROWS=50)
      integer pid(MAX_NUM_ROWS)
      logical accept(MAX_NUM_ROWS)

      integer i, j

      integer algo_eb_filter, algo_inc_kin ! FIXME: typedef this?

c     ------------------------------------------------------------------
c     open the input file
c     ------------------------------------------------------------------

c     parse arguments
      argc = iargc()
      if(argc.lt.1) then
        print *, 'ERROR: please at least specify a HIPO_FILE'
        print *, ''
        print *, 'ARGS: ', 'HIPO_FILE', ' ', 'NUM_EVENTS'
        print *, '  HIPO_FILE: ', 'the input HIPO file'
        print *, '  NUM_EVENTS: ', 'the number of events (0 for all)'
        stop
      else
        call getarg(1, in_file)
      end if
      if(argc.ge.2) then
        call getarg(2, num_events_arg)
        read(num_events_arg,*) num_events
      end if
      print *, 'HIPO_FILE: ', trim(in_file)
      print *, 'NUM_EVENTS: ', num_events

c     open the input HIPO file
      call hipo_file_open(trim(in_file)) ! `trim` removes trailing space
      reader_status = 0
      counter       = 0

c     ------------------------------------------------------------------
c     create iguana algorithms
c     ------------------------------------------------------------------

c     before anything for Iguana, call `iguana_create()`; when done, you
c     must also call `iguana_destroy()` to deallocate the memory
      call iguana_create()
c     enable verbose printouts for the "C-bindings", the C code that
c     allows this Fortran code to use the underlying C++ Iguana code;
c     this is not required, but may be useful if things go wrong...
      call iguana_bindings_set_verbose()
c     then create the algorithms
      call iguana_algo_create(
     &  algo_eb_filter,
     &  'clas12::EventBuilderFilter')
      call iguana_algo_create(
     &  algo_inc_kin,
     &  'physics::InclusiveKinematics')
c     the `algo_` variables are "algorithm indices" (integers); they
c     reference the Iguana algorithms, and you'll need these indices
c     to call algorithm functions (namely, "action functions")
      print *, 'algo_eb_filter: ', algo_eb_filter
      print *, 'algo_inc_kin: ', algo_inc_kin

c     ------------------------------------------------------------------
c     configure and start iguana algorithms
c     ------------------------------------------------------------------

c     set log levels
      call iguana_algo_set_log_level(algo_eb_filter,'debug')
      call iguana_algo_set_log_level(algo_inc_kin,'debug')

c     start algorithms (which "locks" their configuration)
      call iguana_algo_start(algo_eb_filter)
      call iguana_algo_start(algo_inc_kin)

c     ============================================
c     FIXME: why is the log level not being set?
c     ============================================




c       !event loop
c     do while(reader_status.eq.0 .and.
c    &  (num_events.eq.0 .or. counter.lt.num_events))

c         !read banks
c     call hipo_file_next(reader_status)
c     call hipo_read_bank('REC::Particle', nrows)
c     call hipo_read_int('REC::Particle', 'pid', nr, pid, 50)

c         !call iguana filter
c     do i=1, nrows
c     accept(i) = iguana_clas12_EventBuilderFilter_Filter(
c    &event_builder_filter, pid(i))
c     end do

c         !print results
c     write(*,*) '>>>>>>> event ', counter
c     write(*,*) '        nrows ', nrows
c     write(*,*) (pid(j), j=1, nrows)
c     write(*,*) (accept(j), j=1, nrows)

c     counter = counter + 1
c     end do

c       !stop and destroy iguana algorithms
c     call iguana_algo_stop_and_destroy(event_builder_filter)
c     call iguana_algo_stop_and_destroy(momentum_corrections)





c     don't forget to call `iguana_destroy()` when done with Iguana
      call iguana_destroy()

      end program
