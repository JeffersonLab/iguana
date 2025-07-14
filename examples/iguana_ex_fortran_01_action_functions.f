      !> @begin_doc_example{fortran}
      !> @file iguana_ex_fortran_01_action_functions.f
      !> @brief Fortran example demonstrating how to read a HIPO file and use
      !> its data with Iguana algorithms' action functions.
      !> Run with no arguments to see the usage guide.
      !> @see @ref fortran_usage_guide for more guidance
      !> @end_doc_example
      !>
      !> Fortran example program
      program iguana_ex_fortran_01_action_functions

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
      character*1024 config_file ! YAML configuration file
      character*16   num_events_arg
      logical        config_file_set
      character(kind=c_char, len=1024)
     &   in_file_cstr, config_file_cstr, etc_dir

c     HIPO and bank variables
      integer        counter        ! event counter
      integer(c_int) reader_status  ! hipo event loop vars
      integer(c_int) nr             ! unused
      integer        N_MAX          ! max number of rows we can read
      parameter      (N_MAX=50)

c     REC::Particle columns
      integer(c_int) nrows_p ! number of rows
      integer(c_int) pindex(N_MAX)
      integer(c_int) pid(N_MAX)
      real(c_float)  px(N_MAX), py(N_MAX), pz(N_MAX)
      real(c_float)  vz(N_MAX)
      integer(c_int) stat(N_MAX)

c     RUN::config columns
      integer(c_int) nrows_c ! number of rows
      real(c_float)  torus(N_MAX)
      integer(c_int) runnum(N_MAX)

c     REC::Track, REC::Calorimeter, REC::Scintillator columns
      integer(c_int) nrows_trk, nrows_cal, nrows_sci
      integer(c_int) sector_trk(N_MAX) ! sectors from REC::Track
      integer(c_int) pindex_trk(N_MAX) ! pindices from REC::Track
      integer(c_int) sector_cal(N_MAX) ! from REC::Calorimeter
      integer(c_int) pindex_cal(N_MAX)
      integer(c_int) sector_sci(N_MAX) ! from REC::Scintillator
      integer(c_int) pindex_sci(N_MAX)

c     iguana algorithm outputs
      logical(c_bool) accept(N_MAX)  ! filter
      real(c_double) qx, qy, qz, qE  ! q vector
      real(c_double) Q2, x, y, W, nu ! inclusive kinematics
      real(c_double) beamPz, targetM ! beam and target
      integer(c_int) key_vz_filter   ! key for Z-vertex filter
      integer(c_int) key_inc_kin     ! key for inclusive kinematics
      integer(c_int) sector(N_MAX)   ! sectors
      integer(c_int) nrows_sec       ! size of `sector`

c     iguana algorithm indices
      integer(c_int) algo_eb_filter, algo_vz_filter,
     &  algo_inc_kin, algo_sec_finder, algo_mom_cor

c     misc.
      integer i, j
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
        etc_dir = ''
        call iguana_getconfiginstallationprefix(etc_dir)
        print *, 'ERROR: please at least specify a HIPO_FILE'
        print *, ''
        print *, 'ARGS: ', 'HIPO_FILE', ' ', 'NUM_EVENTS'
        print *, ''
        print *, '  HIPO_FILE: ', 'the input HIPO file'
        print *, ''
        print *, '  NUM_EVENTS (optional): ',
     &    'the number of events (0 for all)'
        print *, '    default: ', num_events
        print *, ''
        print *, '  CONFIG_FILE (optional): ',
     &    'algorithm configuration file'
        print *, '    default: ', 'use the internal defaults'
        print *, '    example config file: ',
     &    trim(etc_dir)//'/examples/my_z_vertex_cuts.yaml'
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

c     before anything for Iguana, call `iguana_create()`
      call iguana_create()
!     call iguana_bindings_set_verbose() ! enable additional log print

c     then create the algorithm instances
c     - the 1st argument is an integer, the algorithm index, which
c       references the created instance of the algorithm; it will be
c       set after calling this subroutine and you will need it to call
c       other iguana subroutines (namely, "action functions")
c     - the 2nd argument is the algorithm name
c       IMPORTANT: any time you pass a string to iguana, be sure that
c                  it has the correct termination by appending:
c                  `//c_null_char`
      call iguana_algo_create(
     &  algo_eb_filter,
     &  'clas12::EventBuilderFilter'//c_null_char)
      call iguana_algo_create(
     &  algo_vz_filter,
     &  'clas12::ZVertexFilter'//c_null_char)
      call iguana_algo_create(
     &  algo_inc_kin,
     &  'physics::InclusiveKinematics'//c_null_char)
      call iguana_algo_create(
     &  algo_sec_finder,
     &  'clas12::SectorFinder'//c_null_char)
      call iguana_algo_create(
     &  algo_mom_cor,
     &  'clas12::MomentumCorrection'//c_null_char)

c     ------------------------------------------------------------------
c     configure and start iguana algorithms
c     ------------------------------------------------------------------

c     set log levels (don't forget `//c_null_char`)
      call iguana_algo_set_log_level(
     &  algo_eb_filter, 'debug'//c_null_char)
      call iguana_algo_set_log_level(
     &  algo_vz_filter, 'debug'//c_null_char)
      call iguana_algo_set_log_level(
     &  algo_inc_kin, 'debug'//c_null_char)
      call iguana_algo_set_log_level(
     &  algo_sec_finder, 'debug'//c_null_char)
      call iguana_algo_set_log_level(
     &  algo_mom_cor, 'debug'//c_null_char)

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

        print *, ''
        print *, '>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> event ', counter

c       read banks
        call hipo_file_next(reader_status)
c       get number of rows
        call hipo_read_bank('REC::Particle', nrows_p)
        call hipo_read_bank('RUN::config', nrows_c)
        call hipo_read_bank('REC::Track', nrows_trk)
        call hipo_read_bank('REC::Calorimeter', nrows_cal)
        call hipo_read_bank('REC::Scintillator', nrows_sci)
c       get bank elements
        call hipo_read_int('REC::Particle', 'pid', nr, pid, N_MAX)
        call hipo_read_float('REC::Particle', 'px', nr, px, N_MAX)
        call hipo_read_float('REC::Particle', 'py', nr, py, N_MAX)
        call hipo_read_float('REC::Particle', 'pz', nr, pz, N_MAX)
        call hipo_read_float('REC::Particle', 'vz', nr, vz, N_MAX)
        call hipo_read_int('REC::Particle', 'status', nr, stat, N_MAX)
        call hipo_read_float('RUN::config', 'torus', nr, torus, N_MAX)
        call hipo_read_int('RUN::config', 'run', nr, runnum, N_MAX)
        call hipo_read_int('REC::Track', 'sector', nr,
     &      sector_trk, N_MAX)
        call hipo_read_int('REC::Track', 'pindex', nr,
     &      pindex_trk, N_MAX)
        call hipo_read_int('REC::Calorimeter', 'sector', nr,
     &      sector_cal, N_MAX)
        call hipo_read_int('REC::Calorimeter', 'pindex', nr,
     &      pindex_cal, N_MAX)
        call hipo_read_int('REC::Scintillator', 'sector', nr,
     &      sector_sci, N_MAX)
        call hipo_read_int('REC::Scintillator', 'pindex', nr,
     &      pindex_sci, N_MAX)
c       simple list of pindices in REC::Particle
        do i=1, nrows_p
          pindex(i) = i
        enddo

c       before using the Z-vertext filter, we must "prepare" the
c       algorithm's configuration for this event; the resulting
c       'key_vz_filter' must be passed to the action function;
        call iguana_clas12_zvertexfilter_prepareevent(
     &      algo_vz_filter, runnum(1), key_vz_filter)
c       similarly for the inclusive kinematics algorithm; use '-1'
c       for the beam energy, so RCDB is used to get the energy
        call iguana_physics_inclusivekinematics_prepareevent(
     &      algo_inc_kin, runnum(1), -1, key_inc_kin)

c       call iguana filters
c       - the `logical` variable `accept` must be initialized to
c         `.true.`, since we will use it to "chain" the filters
c       - the event builder filter trivial: by default it accepts only
c         `REC::Particle::pid == 11 or -211` (simple example algorithm)
c       - the AND with the z-vertex filter is the final filter, `accept`
        print *, '===> filter particles with iguana:'
        do i=1, nrows_p
          accept(i) = .true.
          call iguana_clas12_eventbuilderfilter_filter(
     &      algo_eb_filter, pid(i), accept(i))
          call iguana_clas12_zvertexfilter_filter(
     &      algo_vz_filter, vz(i), pid(i), stat(i),
     &      key_vz_filter, accept(i))
          print *, '  i = ', i, '  pid = ', pid(i), ' vz = ', vz(i),
     &      '  status = ', stat(i), '  =>  accept = ', accept(i)
        enddo

c       get sector number
        print *, '>>> debug'
        write(*,*) '  sector_trk', (sector_trk(j),j=1,nrows_trk)
        write(*,*) '  pindex_trk', (pindex_trk(j),j=1,nrows_trk)
        write(*,*) '  sector_cal', (sector_cal(j),j=1,nrows_cal)
        write(*,*) '  pindex_cal', (pindex_cal(j),j=1,nrows_cal)
        write(*,*) '  sector_sci', (sector_sci(j),j=1,nrows_sci)
        write(*,*) '  pindex_sci', (pindex_sci(j),j=1,nrows_sci)
        print *, '<<<'
        call iguana_clas12_sectorfinder_getstandardsectorvec(
     &    algo_sec_finder,
     &    sector_trk, nrows_trk, pindex_trk, nrows_trk,
     &    sector_cal, nrows_cal, pindex_cal, nrows_cal,
     &    sector_sci, nrows_sci, pindex_sci, nrows_sci,
     &    pindex, nrows_p,
     &    sector, nrows_sec)
        print *, '>>> debug'
        write(*,*) '  nrows_sec: ', nrows_sec
        write(*,*) '  sector', (sector(j),j=1,nrows_sec)
        print *, '<<<'

c       momentum corrections
        if(nrows_c.lt.1) then
          print *, '===> no RUN::config bank; cannot ',
     &      'apply momentum corrections'
        else
          print *, '===> momentum corrections:'
          do i=1, nrows_p
            if(accept(i)) then
              print *, '  i = ', i
              print *, '  before: p = (', px(i), py(i), pz(i), ')'
              call iguana_clas12_momentumcorrection_transform(
     &          algo_mom_cor,
     &          px(i), py(i), pz(i),
     &          sector(i), pid(i), torus(1),
     &          px(i), py(i), pz(i))
              print *, '   after: p = (', px(i), py(i), pz(i), ')'
            endif
          enddo
        endif

c       simple electron finder: trigger and highest |p|
        p_ele = 0
        found_ele = .false.
        print *, '===> finding electron...'
        do i=1, nrows_p
          if(accept(i)) then
            print *, '  i = ', i, '     status = ', stat(i)
            if(pid(i).eq.11 .and. stat(i).lt.0) then
              p = sqrt(px(i)**2 + py(i)**2 + pz(i)**2)
              if(p.gt.p_ele) then
                i_ele = i
                p_ele = p
                found_ele = .true.
              endif
            endif
          endif
        enddo
        if(found_ele) then
          print *, '===> found DIS electron:'
          print *, '  i = ', i_ele
          print *, '  p = ', p_ele
        else
          print *, '===> no DIS electron'
        endif

c       compute DIS kinematics with iguana, if electron is found
        if(found_ele) then
          call iguana_physics_inclusivekinematics_computefromlepton(
     &      algo_inc_kin,
     &      px(i_ele), py(i_ele), pz(i_ele),
     &      key_inc_kin,
     &      qx, qy, qz, qE,
     &      Q2, x, y, W, nu,
     &      beamPz, targetM)
          print *, '===> inclusive kinematics:'
          print *, '  q = (', qx, qy, qz, qE, ')'
          print *, ' Q2 = ', Q2
          print *, '  x = ', x
          print *, '  y = ', y
          print *, '  W = ', W
          print *, ' nu = ', nu
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
