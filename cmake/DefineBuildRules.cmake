set( CF_LIBRARY_LINK_FLAGS "" CACHE STRING "Extra link flags for libraries" FORCE )
mark_as_advanced( CF_LIBRARY_LINK_FLAGS )

########################################################################################
# UNIX
########################################################################################

if(UNIX)
  
  # gnu specific warning flags
  if( CMAKE_COMPILER_IS_GNUCC )

    # use pipe for faster compilation
    coolfluid_add_c_flags("-pipe")
    coolfluid_add_cxx_flags("-pipe")
    
    # respect c 89 standard (same as -std=c89)
    coolfluid_add_c_flags("-ansi")
    # respect c++ 98 standard
    coolfluid_add_cxx_flags("-std=c++98")
    # dont allow gnu extensions
    coolfluid_add_cxx_flags("-fno-gnu-keywords")
    
    # dont define common variables
    coolfluid_add_c_flags("-fno-common")
    coolfluid_add_cxx_flags("-fno-common")

    coolfluid_add_cxx_flags("-Wall")
    
    if( CF_ENABLE_WARNINGS )
      # use many warnings
      coolfluid_add_cxx_flags("-W")
      coolfluid_add_cxx_flags("-Wextra")
      coolfluid_add_cxx_flags("-Woverloaded-virtual")
      coolfluid_add_cxx_flags("-Wsign-promo")
      coolfluid_add_cxx_flags("-Wformat")
      # Warn if an undefined identifier is evaluated in an #if directive.
      coolfluid_add_cxx_flags("-Wundef" )
      #  Warn about anything that depends on the "size of" a function type or of "void"
      coolfluid_add_cxx_flags("-Wpointer-arith")
      #  warn about uses of format functions that represent possible security problems
      coolfluid_add_cxx_flags("-Wformat-security")

      # accept functions that dont use all parameters, due to virtual functions may not need all
      coolfluid_add_cxx_flags("-Wno-unused-parameter")
      coolfluid_add_cxx_flags("-Wno-missing-field-initializers")

      # Don't warn when using functors in boost::bind
      coolfluid_add_cxx_flags("-Wno-strict-aliasing")
      # this is temporary until we all move to using openmpi
      # must turn off non-virtual-dtor because many mpi implementations use it
      # KDE uses -Wnon-virtual-dtor
      coolfluid_add_cxx_flags("-Wno-non-virtual-dtor")
      # must turn long long off because many mpi implementations use it
      coolfluid_add_cxx_flags("-Wno-long-long")
      # be pedantic but issue warnings instead of errors
      # coolfluid_add_cxx_flags("-pedantic") # Disabled for now, see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=33305
      coolfluid_add_cxx_flags("-fpermissive")


      coolfluid_add_cxx_flags("-Wno-empty-body")    # Problem in boost
      coolfluid_add_cxx_flags("-Wno-uninitialized") # Problem with boost accumulators

      # could add even these
      #
    endif()

    if( CF_ENABLE_CODECOVERAGE )

      find_program(CTEST_COVERAGE_COMMAND gcov)

      if( CTEST_COVERAGE_COMMAND )
        set( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage" )
        set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage" )
        set( LINK_FLAGS        "${LINK_FLAGS} -fprofile-arcs -ftest-coverage" )
      endif()
    endif()

  endif()

endif(UNIX)

########################################################################################
# WINDOWS
########################################################################################

if(WIN32)

  # stupid VS2005 warning about not using fopen
  add_definitions( -D_CRT_SECURE_NO_DEPRECATE )
  # for M_PI in cmath
  add_definitions( -D_USE_MATH_DEFINES )
  # disable auto-linking with boost
  add_definitions( -DBOOST_ALL_NO_LIB )
  add_definitions( -DBOOST_ALL_DYN_LINK )
  # Required for auto link not to mess up on vs80.
  # @todo Disable auto link on windows altogether.
  # add_definitions( -DBOOST_DYN_LINK )

  # compilation flags
  #   /MD use the Multithreaded DLL of runtime library
  coolfluid_add_c_flags( "/MD" )
  coolfluid_add_cxx_flags( "/MD" )

  # add exception handling
  coolfluid_add_c_flags( "/EHsc" )
  coolfluid_add_cxx_flags( "/EHsc" )

  # linker flags:
  #   /OPT:NOREF keeps functions and data that are never referenced ( needed for static libs )
  set( CF_LIBRARY_LINK_FLAGS "/OPT:NOREF /OPT:NOICF"  CACHE STRING "Extra link flags for libraries" FORCE )

  #   set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /OPT:NOREF /OPT:NOICF" )
  #   set( CMAKE_CXX_CREATE_STATIC_LIBRARY  "lib ${CMAKE_CL_NOLOGO} /OPT:NOREF /OPT:NOICF <LINK_FLAGS> /out:<TARGET> <OBJECTS>" )

endif(WIN32)

########################################################################################
# APPLE
########################################################################################

if( APPLE )

	# improve the linker compiler to avoid unresolved symbols causing errors
  # not needed anymore because all lib depencies are explicitly set
  #  set(CMAKE_CXX_CREATE_SHARED_LIBRARY
  #  "<CMAKE_CXX_COMPILER> -undefined dynamic_lookup <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> <LINK_FLAGS> -o <TARGET> -install_name <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")

  # under Mac OS X internal deps must be used so force them
  if( NOT CF_ENABLE_INTERNAL_DEPS )
    set( CF_ENABLE_INTERNAL_DEPS ON CACHE BOOL "Use of internal deps is forced" FORCE )
  endif()

endif()

########################################################################################
# GENERIC
########################################################################################

# Disable boost pre-1.34 boost::filesystem functions.
# add_definitions ( -DBOOST_FILESYSTEM_NO_DEPRECATED )

########################################################################################
# FINAL
########################################################################################

# test and add the user defined flags

STRING ( REGEX MATCHALL "[^ ]+" C_FLAGS_LIST "${CF_C_FLAGS}"  )
foreach( c_flag ${C_FLAGS_LIST} )
  coolfluid_add_c_flags_SIGNAL_ERROR ( ${c_flag} )
endforeach()
mark_as_advanced( C_FLAGS_LIST   )

STRING ( REGEX MATCHALL "[^ ]+" CXX_FLAGS_LIST "${CF_CXX_FLAGS}"  )
foreach( cxx_flag ${CXX_FLAGS_LIST} )
  coolfluid_add_cxx_flags_SIGNAL_ERROR ( ${cxx_flag} )
endforeach()
mark_as_advanced( CXX_FLAGS_LIST  )

if( NOT CF_SKIP_FORTRAN )
  STRING ( REGEX MATCHALL "[^ ]+" Fortran_FLAGS_LIST "${CF_Fortran_FLAGS}"  )
  # fortran flags currently nont checked
  set( CMAKE_Fortran_FLAGS "${CF_Fortran_FLAGS}" )
  # foreach( fortran_flag ${Fortran_FLAGS_LIST} )
  #   CF_ADD_Fortran_FLAGS_SIGNAL_ERROR ( ${fortran_flag} )
  # endforeach()
  mark_as_advanced( Fortran_FLAGS_LISTS )
endif()

