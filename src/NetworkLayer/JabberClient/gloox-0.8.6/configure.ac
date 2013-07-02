dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT
AC_CONFIG_SRCDIR([configure.ac])
dnl Every other copy of the package version number gets its value from here
AM_INIT_AUTOMAKE(gloox,0.8.6)

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h)

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_LIBTOOL

dnl
dnl extra paths
dnl

AC_ARG_WITH(extra_include_path, AC_HELP_STRING([--with-extra-include-path],
                                           [use additional include paths]),
            extra_include_path=$withval)
split_includes="`echo $extra_include_path | sed -e 's/:/ /g'`"
for incpath in $split_includes ; do
    CPPFLAGS="-I$incpath $CPPFLAGS"
done

AC_ARG_WITH(extra_library_path, AC_HELP_STRING([--with-extra-library-path],
                                               [use additional library paths]),
            extra_library_path=$withval)
split_libs="`echo $extra_library_path | sed -e 's/:/ /g'`"
for libpath in $split_libs ; do
    LDFLAGS="-L$libpath $LDFLAGS"
done

dnl Checks for libraries.

dnl Iksemel
AC_CHECK_HEADER(iksemel.h,
AC_CHECK_LIB(iksemel, iks_has_tls,
         [libiksemel=yes LIBS="${LIBS} -liksemel"], libiksemel=no),
         libiksemel=no)
if test "$libiksemel" == "no" ; then
  AC_MSG_ERROR([Libiksemel not found])
fi
AC_MSG_CHECKING([if Libiksemel is present])
AC_MSG_RESULT($libiksemel)


dnl Libidn
AC_ARG_WITH(libidn, AC_HELP_STRING([--with-libidn=[DIR]],
      [Support IDN (needs GNU Libidn)]),
      libidn=$withval, libidn=yes)
if test "$libidn" != "no"; then
  if test "$libidn" != "yes"; then
    LDFLAGS="${LDFLAGS} -L$libidn/lib"
    CPPFLAGS="${CPPFLAGS} -I$libidn/include"
  fi
  AC_CHECK_HEADER(idna.h,
  AC_CHECK_LIB(idn, stringprep_check_version,
           [libidn=yes LIBS="${LIBS} -lidn"], libidn=no),
           libidn=no)
fi
if test "$libidn" != "no" ; then
  AC_DEFINE(HAVE_LIBIDN, 1, [Define to 1 if you want IDN support.])
else
  AC_MSG_WARN([Libidn not found])
fi
AC_MSG_CHECKING([if Libidn should be used])
AC_MSG_RESULT($libidn)

dnl GnuTLS
AC_ARG_WITH(gnutls, AC_HELP_STRING([--with-gnutls=[DIR]],
      [Support Stream Encryption using GnuTLS]),
      gnutls=$withval, gnutls=yes)
if test "$gnutls" != "no"; then
  if test "$gnutls" != "yes"; then
    LDFLAGS="${LDFLAGS} -L$gnutls/lib"
    CPPFLAGS="${CPPFLAGS} -I$gnutls/include"
    gnutls_path="$gnutls/bin/"
  fi
  AC_CHECK_HEADER(gnutls/gnutls.h,
  AC_CHECK_LIB(gnutls, gnutls_check_version,
           [gnutls=yes LIBS="${LIBS} `${gnutls_path}libgnutls-config --libs`"], gnutls=no),
           gnutls=no)
fi
if test "$gnutls" != "no" ; then
  AC_DEFINE(HAVE_GNUTLS, 1, [Define to 1 if you want TLS support (GnuTLS). Undefine HAVE_OPENSSL.])
else
  AC_MSG_WARN([GnuTLS not found])
fi
AC_MSG_CHECKING([if GnuTLS should be used])
AC_MSG_RESULT($gnutls)

dnl OpenSSL
if test "$gnutls" == "no"; then
  AC_ARG_WITH(openssl, AC_HELP_STRING([--with-openssl=[DIR]],
        [Support Stream Encryption using OpenSSL]),
        openssl=$withval, openssl=yes)
  if test "$openssl" != "no"; then
    if test "$openssl" != "yes"; then
      LDFLAGS="${LDFLAGS} -L$openssl/lib"
      CPPFLAGS="${CPPFLAGS} -I$openssl/include"
    fi
dnl OpenSSL on MinGW requires additional Windows libraries to compile
    case "$host_os" in
      *mingw*)
         SSL_LIBS="-lcrypto -lwsock32 -lgdi32";;
      *)
         SSL_LIBS="-lcrypto";;
    esac
    LIBS_TMP=${LIBS}
    LIBS="${LIBS} -lssl ${SSL_LIBS}"
    AC_CHECK_HEADER(openssl/ssl.h,
    AC_CHECK_LIB(ssl, SSL_library_init,
            [openssl=yes], openssl=no),
            [openssl=no LIBS=${TMP_LIBS}])
  fi
  if test "$openssl" != "no" ; then
    AC_DEFINE(HAVE_OPENSSL, 1, [Define to 1 if you want TLS support (OpenSSL).])
  else
    AC_MSG_WARN([OpenSSL not found])
  fi
  AC_MSG_CHECKING([if OpenSSL should be used])
  AC_MSG_RESULT($openssl)
fi

dnl Zlib
AC_ARG_WITH(zlib, AC_HELP_STRING([--with-zlib=[DIR]],
      [Support Stream Compression (needs Zlib)]),
      zlib=$withval, zlib=yes)
if test "$zlib" != "no"; then
  if test "$zlib" != "yes"; then
    LDFLAGS="${LDFLAGS} -L$zlib/lib"
    CPPFLAGS="${CPPFLAGS} -I$zlib/include"
  fi
  AC_CHECK_HEADER(zlib.h,
  AC_CHECK_LIB(z, zlibVersion,
           [zlib=yes LIBS="${LIBS} -lz"], zlib=no),
           zlib=no)
fi
if test "$zlib" != "no" ; then
  AC_DEFINE(HAVE_ZLIB, 1, [Define to 1 if you want Stream Compression support.])
else
  AC_MSG_WARN([Zlib not found])
fi
AC_MSG_CHECKING([if Zlib should be used])
AC_MSG_RESULT($zlib)

dnl check for res_querydomain in libc, libbind and libresolv
AC_CHECK_FUNCS(res_querydomain)
if test "x-$ac_cv_func_res_querydomain" = "x-yes" ; then
  have_res_querydomain=yes
else
  AC_CHECK_LIB(resolv, res_querydomain)
  if test "x-$ac_cv_lib_resolv_res_querydomain" = "x-yes" ; then
    have_res_querydomain=yes
    AC_DEFINE(HAVE_RES_QUERYDOMAIN)
  else
    AC_CHECK_LIB(bind, res_querydomain)
    if test "x-$ac_cv_lib_bind_res_querydomain" = "x-yes" ; then
      have_res_querydomain=yes
      AC_DEFINE(HAVE_RES_QUERYDOMAIN)
    else
      AC_MSG_CHECKING([for res_querydomain in -lresolv (alternate version)])
      save_libs="$LIBS"
      LIBS="-lresolv $LIBS"
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <resolv.h>]], [[res_querydomain(0,0,0,0,0,0)]])],
                     [AC_MSG_RESULT(yes)
                      AC_DEFINE(HAVE_RES_QUERYDOMAIN)
                      have_res_querydomain=yes],
                     [AC_MSG_RESULT(no)
                     LIBS="$save_libs"])
    fi
  fi
fi

dnl check for dn_skipname in libc, libbind and libresolv
AC_CHECK_FUNCS(dn_skipname)
if test "x-$ac_cv_func_dn_skipname" = "x-yes" ; then
  have_dn_skipname=yes
else
  AC_CHECK_LIB(resolv, dn_skipname)
  if test "x-$ac_cv_lib_resolv_dn_skipname" = "x-yes" ; then
    have_dn_skipname=yes
    AC_DEFINE(HAVE_DN_SKIPNAME)
  else
    AC_CHECK_LIB(bind, dn_skipname)
    if test "x-$ac_cv_lib_bind_dn_skipname" = "x-yes" ; then
      have_dn_skipname=yes
      AC_DEFINE(HAVE_DN_SKIPNAME)
    else
      AC_MSG_CHECKING([for dn_skipname in -lresolv (alternate version)])
      save_libs="$LIBS"
      LIBS="-lresolv $LIBS"
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <netinet/in.h>
 #include <resolv.h>]], [[dn_skipname(0,0)]])],
                     [AC_MSG_RESULT(yes)
                      AC_DEFINE(HAVE_DN_SKIPNAME)
                      have_dn_skipname=yes],
                     [AC_MSG_RESULT(no)
                     LIBS="$save_libs"])
    fi
  fi
fi

dnl check for res_query in libc, libbind and libresolv
AC_CHECK_FUNCS(res_query)
if test "x-$ac_cv_func_res_query" = "x-yes" ; then
  have_res_query=yes
else
  AC_CHECK_LIB(resolv, res_query)
  if test "x-$ac_cv_lib_resolv_res_query" = "x-yes" ; then
    have_res_query=yes
    AC_DEFINE(HAVE_RES_QUERY)
  else
    AC_CHECK_LIB(bind, res_query)
    if test "x-$ac_cv_lib_bind_res_query" = "x-yes" ; then
      have_res_query=yes
      AC_DEFINE(HAVE_RES_QUERY)
    else
      AC_MSG_CHECKING([for res_query in -lresolv (alternate version)])
      save_libs="$LIBS"
      LIBS="-lresolv $LIBS"
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <resolv.h>]], [[res_query(0,0)]])],
                     [AC_MSG_RESULT(yes)
                      AC_DEFINE(HAVE_RES_QUERY)
                      have_res_query=yes],
                     [AC_MSG_RESULT(no)
                     LIBS="$save_libs"])
    fi
  fi
fi

dnl check for STLport
AC_DEFUN([STLPORT_CHECK],
[AC_CACHE_CHECK([STLport is usable], ac_cv_stlport,
[
  tmp_cpp_flags="$CPPFLAGS"
  tmp_libs="$LIBS"
  CPPFLAGS="-I${ac_stlport_path}/stlport $CPPFLAGS -pthread"
  LIBS="-L${ac_stlport_path}/lib -Wl,-rpath=${ac_stlport_path}/lib -lstlport $LIBS"
  AC_TRY_RUN(
  [#include <cstdio>
  int main()
{
#ifdef STLPORT
    return 0;
#else
    return 1;
#endif
}],
  ac_cv_stlport=yes,
  ac_cv_stlport=no)
  CPPFLAGS="$tmp_cpp_flags"
  LIBS="$tmp_libs"
])
  if test $ac_cv_stlport = yes; then
    CPPFLAGS="-I${ac_stlport_path}/stlport $CPPFLAGS -pthread"
    LIBS="-L${ac_stlport_path}/lib -Wl,-rpath=${ac_stlport_path}/lib -lstlport $LIBS"
  fi
])
AC_ARG_WITH(stlport,[  --with-stlport=@<:@DIR@:>@          use STLport installed in DIR (default: not used)],
                      [ac_stlport_path=$withval STLPORT_CHECK],
                      [ac_cv_stlport=no])

dnl Debug
debug="no"
AC_ARG_ENABLE( debug,
               [  --enable-debug          turn on debugging [default=no]],
               [debug="yes"] )
if test "x$debug" = "xyes"; then
      CPPFLAGS="$CPPFLAGS -g3 -fno-inline -DDEBUG"
fi
AC_MSG_CHECKING([whether to enable debug])
AC_MSG_RESULT($debug)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h strings.h errno.h arpa/nameser.h)
AC_CHECK_FUNCS(ns_get16 __ns_get16)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

dnl Generating makefiles.
AC_CONFIG_FILES([
Makefile
gloox.pc
gloox-config
src/Makefile
src/tests/Makefile
])
AC_OUTPUT
