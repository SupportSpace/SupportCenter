# readline.m4 serial 5
dnl Copyright (C) 2005, 2006 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Written by Simon Josefsson, with help from Bruno Haible and Oskar
dnl Liljeblad.

AC_DEFUN([gl_FUNC_READLINE],
[
  dnl Prerequisites of AC_LIB_LINKFLAGS_BODY.
  AC_REQUIRE([AC_LIB_PREPARE_PREFIX])
  AC_REQUIRE([AC_LIB_RPATH])

  dnl Search for libreadline and define LIBREADLINE, LTLIBREADLINE and
  dnl INCREADLINE accordingly.
  AC_LIB_LINKFLAGS_BODY([readline])

  dnl Add $INCREADLINE to CPPFLAGS before performing the following checks,
  dnl because if the user has installed libreadline and not disabled its use
  dnl via --without-libreadline-prefix, he wants to use it. The AC_TRY_LINK
  dnl will then succeed.
  am_save_CPPFLAGS="$CPPFLAGS"
  AC_LIB_APPENDTOVAR([CPPFLAGS], [$INCREADLINE])

  AC_CACHE_CHECK(for readline, gl_cv_lib_readline, [
    gl_cv_lib_readline=no
    am_save_LIBS="$LIBS"
    dnl On some systems, -lreadline doesn't link without an additional
    dnl -lncurses or -ltermcap.
    dnl Try -lncurses before -ltermcap, because libtermcap is unsecure
    dnl by design and obsolete since 1994. Try -lcurses last, because
    dnl libcurses is unusable on some old Unices.
    for extra_lib in "" ncurses termcap curses; do
      LIBS="$am_save_LIBS $LIBREADLINE"
      if test -n "$extra_lib"; then
        LIBS="$LIBS -l$extra_lib"
      fi
      AC_TRY_LINK([#include <stdio.h>
#include <readline/readline.h>],
        [readline((char*)0);],
        [gl_cv_lib_readline=" -l$extra_lib"])
      if test "$gl_cv_lib_readline" != no; then
	break
      fi
    done
    LIBS="$am_save_LIBS"
  ])

  if test "$gl_cv_lib_readline" != no; then
    AC_DEFINE(HAVE_READLINE, 1, [Define if you have the readline library.])
    if test "$gl_cv_lib_readline" != " -l"; then
      LIBREADLINE="$LIBREADLINE$gl_cv_lib_readline"
      LTLIBREADLINE="$LTLIBREADLINE$gl_cv_lib_readline"
    fi
    AC_MSG_CHECKING([how to link with libreadline])
    AC_MSG_RESULT([$LIBREADLINE])
  else
    dnl If $LIBREADLINE didn't lead to a usable library, we don't
    dnl need $INCREADLINE either.
    CPPFLAGS="$am_save_CPPFLAGS"
    LIBREADLINE=
    LTLIBREADLINE=
  fi
  AC_SUBST(LIBREADLINE)
  AC_SUBST(LTLIBREADLINE)

  AC_CHECK_HEADERS(readline/readline.h)

  if test $gl_cv_lib_readline = no; then
    AC_LIBOBJ(readline)
    gl_PREREQ_READLINE
  fi
])

# Prerequisites of lib/readline.c.
AC_DEFUN([gl_PREREQ_READLINE], [
  :
])
