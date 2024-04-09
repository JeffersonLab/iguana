      program iguana_example_fortran_01_bank_rows

        use iso_c_binding ! for string termination

        character(2048) :: in_file
        character(2048) :: num_events_str
        integer :: len
        integer :: num_events = 10

        integer :: reader_status
        integer :: counter
        integer :: nrows
        integer :: nr
        integer :: pid(50)
        integer :: accept(50)

        ! parse arguments
        if(command_argument_count().ge.1) then
          call get_command_argument(1, in_file)
        endif
        if(command_argument_count().ge.2) then
          call get_command_argument(2, num_events_str)
          read(num_events_str,*) num_events
        endif

        ! open the HIPO file
        len = len_trim(in_file)
        in_file(len+1:len+1) = c_null_char
        call hipo_file_open(in_file)

        reader_status = 0
        counter       = 0

        ! initialize iguana algorithm
        call fort_start()

        ! event loop
        do while(reader_status.eq.0)
          if(num_events.gt.0 .and. counter.ge.num_events) exit

          ! read banks
          call hipo_file_next(reader_status)
          call hipo_read_bank('REC::Particle', nrows)
          call hipo_read_int('REC::Particle', 'pid', nr, pid, 50)

          ! call iguana filter
          do 10 i=1, nrows
            call fort_filter(pid(i), accept(i))
 10       continue

          ! print results
          write(*,*) '>>>>>>> event ', counter
          write(*,*) '        nrows ', nrows
          write(*,*) (pid(j), j=1, nrows)
          write(*,*) (accept(j), j=1, nrows)

          counter = counter + 1
        end do

      end program
