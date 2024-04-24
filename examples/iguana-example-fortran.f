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

c     program parameters
      integer*4      argc
      character*1024 in_file ! HIPO file
      character*32   num_events_arg
      integer        num_events / 10 / ! num. events to read (0 = all)

c     HIPO and bank variables
      integer   reader_status, counter ! hipo event loop vars
      integer   nrows ! number of rows in `REC::Particle`
      integer   nr ! number of rows that have been read
      integer   N_MAX ! the maximum number of rows we can read
      parameter (N_MAX=50)

c     REC::Particle columns
      integer pid(N_MAX)
      real    px(N_MAX), py(N_MAX), pz(N_MAX)
      integer stat(N_MAX)

c     iguana algorithm indices
      integer algo_eb_filter, algo_inc_kin

c     misc.
      integer i
      logical accept(N_MAX)
      real    p, p_max
      integer i_ele
      logical found_ele

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
!     call iguana_bindings_set_verbose() ! enable additional log print

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

        print *, '>>>>>>> event ', counter

c       read banks
        call hipo_file_next(reader_status)
        call hipo_read_bank('REC::Particle', nrows)
        call hipo_read_int('REC::Particle',   'pid',    nr, pid,  N_MAX)
        call hipo_read_float('REC::Particle', 'px',     nr, px,   N_MAX)
        call hipo_read_float('REC::Particle', 'py',     nr, py,   N_MAX)
        call hipo_read_float('REC::Particle', 'pz',     nr, pz,   N_MAX)
        call hipo_read_int('REC::Particle',   'status', nr, stat, N_MAX)

c       call iguana filter
        print *, 'PID filter:'
        do 20 i=1, nrows
          call iguana_clas12_EventBuilderFilter_Filter(
     &      algo_eb_filter, pid(i), accept(i))
          print *, '  ', pid(i), '  =>  accept = ', accept(i)
 20     continue

c       simple electron finder: trigger and highest |p|
        p_max = 0
        do 30 i=1, nrows
          if(.not.accept(i)) goto 30
          if(pid(i).eq.11 .and. stat(i).lt.0) then
            p = sqrt(px(i)**2 + py(i)**2 + pz(i)**2)
            if(p.gt.p_max) then
              i_ele = i
              p_max = p
              found_ele = .true.
            endif
          endif
 30     continue

c       compute DIS kinematics, if electron is found
        if(found_ele) then
          print *, '===> found electron'
        else
          print *, '===> no electron'
        endif

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
