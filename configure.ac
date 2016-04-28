AC_INIT([phy905proj], [1.0])
AM_INIT_AUTOMAKE([-Wall -Werror foreign nostdinc])
AX_PROG_CC_MPI()
AX_PROG_CXX_MPI()
AX_CXX_COMPILE_STDCXX_11([], [mandatory])
AC_C_INLINE
AC_TYPE_SIZE_T

AC_CHECK_FUNCS([clock_gettime], [],
    [AC_SEARCH_LIBS([clock_gettime], [rt])])
AC_CHECK_FUNCS([sqrt], [],
    [AC_SEARCH_LIBS([sqrt], [m], [],
        AC_MSG_ERROR([missing sqrt]))])
AC_CHECK_HEADERS([limits.h stddef.h stdlib.h], [],
    AC_MSG_ERROR([missing essential headers]))

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT
