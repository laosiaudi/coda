dnl Process this file with autoconf to produce a configure script

dnl Initialization
AC_INIT(src/lwp.c)
CODA_SETUP_BUILD
AM_INIT_AUTOMAKE(lwp, 1.9)
AM_CONFIG_HEADER(config.h)

dnl For library version updates, follow these steps in order.
dnl - when any library source has changed, increment first
dnl - when interfaces were added/removed changed, increment second and set
dnl   first to 0
dnl - if any interfaces were added, increment third
dnl - if any interfaces were removed, set third to 0
CODA_LIBRARY_VERSION(0, 2, 0)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_RANLIB
AC_CHECK_PROG(AR, ar, ar)
export AR
AC_CHECK_PROG(AS, as, as)
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL

dnl Checks for libraries.
AC_SUBST(LIBPTHREAD)
AC_ARG_ENABLE(pthread,
  [  --enable-pthread        build experimental pthreaded LWP library],
  includepthread="$enableval", includepthread="no")
AC_CHECK_LIB(pthread, pthread_create, [LIBPTHREAD="-lpthread"])
AC_CHECK_LIB(posix4, sem_init, [LIBPTHREAD="${LIBPTHREAD} -lposix4"])
AM_CONDITIONAL(build_pthread, test "$includepthread" != "no" -a "x$LIBPTHREAD" != x)

dnl Checks for header files.

dnl Checks for typedefs.
AC_CHECK_TYPE(register_t, int)

dnl Checks for compiler characteristics.
CODA_DARWIN_BROKEN_CPP_WORKAROUND

dnl Checks for library functions.
AC_FUNC_MMAP

dnl Checks for system services.
AC_CYGWIN
AC_EXEEXT

dnl Generate output files
AC_SUBST(AFLAGS)
AC_OUTPUT(Makefile include/Makefile include/lwp/Makefile src/Makefile src-pt/Makefile pkgs/Makefile pkgs/pkg-bsd.sh pkgs/lwp.spec pkgs/lwp-nt.spec pkgs/lwp-dos.spec pkgs/lwp-win95.spec debian/Makefile)
