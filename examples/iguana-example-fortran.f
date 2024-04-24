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
      call iguana_bindings_set_verbose() ! enable additional log print

c     then create the algorithm instances
c     - the 1st argument is an integer, the algorithm index, which
c       references the created instance of the algorithm; it will be
c       set after calling this subroutine and you will need it to call
c       other iguana subroutines (namely, "action functions")
c     - the 2nd argument is the algorithm name
      call iguana_algo_create(
     &  algo_eb_filter,
     &  'clas12::EventBuilderFilter')
      call iguana_algo_create(
     &  algo_inc_kin,
     &  'physics::InclusiveKinematics')

c     print the algorithm indices
      print *, 'algo_eb_filter: ', algo_eb_filter
      print *, 'algo_inc_kin: ', algo_inc_kin

c     ------------------------------------------------------------------
c     configure and start iguana algorithms
c     ------------------------------------------------------------------

c     set log levels
      call iguana_algo_set_log_level(algo_eb_filter,'debug')
      call iguana_algo_set_log_level(algo_inc_kin,'debug')

c     TODO: show how to configure an algorithm using a config file

c     start all created algorithms, which "locks" their configuration
      call iguana_start()

c     ------------------------------------------------------------------
c     event loop
c     ------------------------------------------------------------------

 10   if(reader_status.eq.0 .and.
     &  (num_events.eq.0 .or. counter.lt.num_events)) then

c       read banks
        call hipo_file_next(reader_status)
        call hipo_read_bank('REC::Particle', nrows)
        call hipo_read_int('REC::Particle', 'pid', nr, pid, 50)

c       call iguana filter
        do 20 i=1, nrows
          call iguana_clas12_EventBuilderFilter_Filter(
     &      algo_eb_filter, pid(i), accept(i))
 20     continue

c       print results
        print *, '>>>>>>> event ', counter
        print *, '        nrows ', nrows
        print *, 'PIDs: ', (pid(j), j=1, nrows)
        print *, 'Accept: ', (accept(j), j=1, nrows)

        counter = counter + 1
        goto 10
      endif

c     ------------------------------------------------------------------
c     cleanup
c     ------------------------------------------------------------------

c     don't forget to call `iguana_stop()` to stop the algorithms
c     and free the allocated memory
      call iguana_stop()

      end program
