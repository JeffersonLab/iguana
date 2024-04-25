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
      use iso_c_binding
      implicit none

c     ------------------------------------------------------------------
c     data declarations
c     ------------------------------------------------------------------
c     NOTE: using `iso_c_binding` types for input and output of C-bound
c           functions and subroutines, i.e., for HIPO and Iguana usage;
c           using standard F77 types may still work, but might be
c           compiler dependent in some cases

c     program parameters
      integer*4      argc
      character*1024 in_file     ! HIPO file
      integer        num_events  ! number of events to read
      character*16   num_events_arg
      character*1024 config_file ! YAML configuration file
      character(kind=c_char, len=1024) in_file_cstr, config_file_cstr
      logical        config_file_set

c     HIPO and bank variables
      integer        counter       ! event counter
      integer(c_int) reader_status ! hipo event loop vars
      integer(c_int) nrows         ! number of rows in `REC::Particle`
      integer(c_int) nr            ! number of rows that have been read
      integer        N_MAX         ! the max number of rows we can read
      parameter      (N_MAX=50)

c     REC::Particle columns
      integer(c_int) pid(N_MAX)
      real(c_float)  px(N_MAX), py(N_MAX), pz(N_MAX)
      real(c_float)  vz(N_MAX)
      integer(c_int) stat(N_MAX)

c     iguana algorithm outputs
      logical(c_bool) accept(N_MAX)  ! filter
      real(c_double) qx, qy, qz, qE  ! q vector
      real(c_double) Q2, x, y, W, nu ! inclusive kinematics

c     iguana algorithm indices
      integer(c_int) algo_eb_filter, algo_vz_filter, algo_inc_kin

c     misc.
      integer i
      real    p, p_ele
      integer i_ele
      logical found_ele

c     ------------------------------------------------------------------
c     open the input file
c     ------------------------------------------------------------------

c     parse arguments
      num_events      = 10
      config_file_set = .false.
      argc            = iargc()
      if(argc.lt.1) then
        print *, 'ERROR: please at least specify a HIPO_FILE'
        print *, ''
        print *, 'ARGS: ', 'HIPO_FILE', ' ', 'NUM_EVENTS'
        print *, ''
        print *, '  HIPO_FILE: ', 'the input HIPO file'
        print *, ''
        print *, '  NUM_EVENTS: ', 'the number of events (0 for all)'
        print *, '    default: ', num_events
        print *, ''
        print *, '  CONFIG_FILE: ', 'algorithm configuration file'
        print *, '    default: ', 'use the internal defaults'
        print *, '    example: ',
     &    '$prefix/etc/iguana/examples/my_z_vertex_cuts.yaml'
        stop
      else
        call getarg(1, in_file)
        in_file_cstr = trim(in_file)//c_null_char
      end if
      if(argc.ge.2) then
        call getarg(2, num_events_arg)
        read(num_events_arg,*) num_events
      end if
      if(argc.ge.3) then
        call getarg(3, config_file)
        config_file_cstr = trim(config_file)//c_null_char
        config_file_set = .true.
      endif

c     open the input HIPO file
      call hipo_file_open(in_file_cstr)
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
     &  algo_vz_filter,
     &  'clas12::ZVertexFilter')
      call iguana_algo_create(
     &  algo_inc_kin,
     &  'physics::InclusiveKinematics')

c     ------------------------------------------------------------------
c     configure and start iguana algorithms
c     ------------------------------------------------------------------

c     set log levels
      call iguana_algo_set_log_level(algo_eb_filter,'debug')
      call iguana_algo_set_log_level(algo_vz_filter,'debug')
      call iguana_algo_set_log_level(algo_inc_kin,'debug')

c     configure algorithms with a configuration file
      if(config_file_set) then
        call iguana_set_config_file(config_file_cstr)
      endif

c     start all algorithms, which "locks" their configuration
      call iguana_start()

c     ------------------------------------------------------------------
c     event loop
c     ------------------------------------------------------------------

 10   if(reader_status.eq.0 .and.
     &  (num_events.eq.0 .or. counter.lt.num_events)) then

        print *, '>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> event ', counter

c       read banks
        call hipo_file_next(reader_status)
        call hipo_read_bank('REC::Particle', nrows)
        call hipo_read_int('REC::Particle',   'pid',    nr, pid,  N_MAX)
        call hipo_read_float('REC::Particle', 'px',     nr, px,   N_MAX)
        call hipo_read_float('REC::Particle', 'py',     nr, py,   N_MAX)
        call hipo_read_float('REC::Particle', 'pz',     nr, pz,   N_MAX)
        call hipo_read_float('REC::Particle', 'vz',     nr, vz,   N_MAX)
        call hipo_read_int('REC::Particle',   'status', nr, stat, N_MAX)

c       call iguana filters
c       - the `logical` variable `accept` must be initialized to
c         `.true.`, since we will use it to "chain" the filters
c       - the event builder filter trivial: by default it accepts only
c         `REC::Particle::pid == 11 or -211` (simple example algorithm)
c       - the AND with the z-vertex filter is the final filter, `accept`
        print *, 'Filter:'
        do 20 i=1, nrows
          accept(i) = .true.
          call iguana_clas12_eventbuilderfilter_filter(
     &      algo_eb_filter, pid(i), accept(i))
          call iguana_clas12_zvertexfilter_filter(
     &      algo_vz_filter, vz(i), accept(i))
          print *, '  ', pid(i), ' vz = ', vz(i),
     &      '  =>  accept = ', accept(i)
 20     continue

c       simple electron finder: trigger and highest |p|
        p_ele = 0
        found_ele = .false.
        do 30 i=1, nrows
          if(.not.accept(i)) goto 30
          if(pid(i).eq.11 .and. stat(i).lt.0) then
            p = sqrt(px(i)**2 + py(i)**2 + pz(i)**2)
            if(p.gt.p_ele) then
              i_ele = i
              p_ele = p
              found_ele = .true.
            endif
          endif
 30     continue
        if(found_ele) then
          print *, '===> found electron'
          print *, '  i: ', i_ele
          print *, '  p: ', p_ele
        else
          print *, '===> no electron'
        endif

c       compute DIS kinematics, if electron is found
        if(found_ele) then
          call iguana_physics_inclusivekinematics_computefromlepton(
     &      algo_inc_kin,
     &      px(i_ele), py(i_ele), pz(i_ele),
     &      qx, qy, qz, qE,
     &      Q2, x, y, W, nu)
          print *, '===> inclusive kinematics:'
          print *, '  q: (', qx, qy, qz, qE, ')'
          print *, ' Q2: ', Q2
          print *, '  x: ', x
          print *, '  y: ', y
          print *, '  W: ', W
          print *, ' nu: ', nu
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
