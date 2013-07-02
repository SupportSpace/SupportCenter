# Copyright (C) 2004-2006 Free Software Foundation, Inc.
#
# This file is free software, distributed under the terms of the GNU
# General Public License.  As a special exception to the GNU General
# Public License, this file may be distributed as part of a program
# that contains a configuration script generated by Autoconf, under
# the same distribution terms as the rest of that program.
#
# Generated by gnulib-tool.
#
# This file represents the compiled summary of the specification in
# gnulib-cache.m4. It lists the computed macro invocations that need
# to be invoked from configure.ac.
# In projects using CVS, this file can be treated like other built files.


# This macro should be invoked from ./configure.in, in the section
# "Checks for programs", right after AC_PROG_CC, and certainly before
# any checks for libraries, header files, types and library functions.
AC_DEFUN([gl_EARLY],
[
  m4_pattern_forbid([^gl_[A-Z]])dnl the gnulib macro namespace
  m4_pattern_allow([^gl_ES$])dnl a valid locale name
  AC_REQUIRE([AC_PROG_RANLIB])
  AC_REQUIRE([AC_GNU_SOURCE])
  AC_REQUIRE([gl_LOCK_EARLY])
])

# This macro should be invoked from ./configure.in, in the section
# "Check for header files, types and library functions".
AC_DEFUN([gl_INIT],
[
  AM_CONDITIONAL([GL_COND_LIBTOOL], [true])
  gl_cond_libtool=true
  gl_FUNC_ALLOCA
  gl_HEADER_ARPA_INET
  gl_GC
  if test $gl_cond_libtool = false; then
    gl_ltlibdeps="$gl_ltlibdeps $LTLIBGCRYPT"
    gl_libdeps="$gl_libdeps $LIBGCRYPT"
  fi
  gl_GC_ARCFOUR
  gl_GC_ARCTWO
  gl_GC_DES
  gl_GC_HMAC_MD5
  gl_GC_HMAC_SHA1
  gl_GC_MD2
  gl_GC_MD4
  gl_GC_MD5
  gl_GC_PBKDF2_SHA1
  gl_GC_RANDOM
  gl_GC_RIJNDAEL
  gl_GC_SHA1
  gl_GETADDRINFO
  gl_FUNC_GETDELIM
  gl_FUNC_GETLINE
  gl_FUNC_GETPASS
  dnl you must add AM_GNU_GETTEXT([external]) or similar to configure.ac.
  AM_GNU_GETTEXT_VERSION([0.15])
  gl_INET_NTOP
  gl_INET_PTON
  gl_MD2
  gl_FUNC_MEMMEM
  gl_FUNC_MEMMOVE
  gl_MINMAX
  gl_HEADER_NETINET_IN
  gl_FUNC_READ_FILE
  gl_FUNC_READLINE
  gl_SIZE_MAX
  gl_FUNC_SNPRINTF
  gl_TYPE_SOCKLEN_T
  AM_STDBOOL_H
  gl_STDINT_H
  gl_FUNC_STRDUP
  gl_HEADER_SYS_SOCKET
  gl_FUNC_VASNPRINTF
  gl_XSIZE
])

# This macro records the list of files which have been installed by
# gnulib-tool and may be removed by future gnulib-tool invocations.
AC_DEFUN([gl_FILE_LIST], [
  build-aux/GNUmakefile
  build-aux/config.rpath
  build-aux/gendocs.sh
  build-aux/maint.mk
  doc/fdl.texi
  doc/gendocs_template
  doc/gpl.texi
  doc/lgpl.texi
  lib/alloca_.h
  lib/arcfour.c
  lib/arcfour.h
  lib/arctwo.c
  lib/arctwo.h
  lib/asnprintf.c
  lib/des.c
  lib/des.h
  lib/dummy.c
  lib/gai_strerror.c
  lib/gc-gnulib.c
  lib/gc-libgcrypt.c
  lib/gc-pbkdf2-sha1.c
  lib/gc.h
  lib/getaddrinfo.c
  lib/getaddrinfo.h
  lib/getdelim.c
  lib/getdelim.h
  lib/getline.c
  lib/getline.h
  lib/getpass.c
  lib/getpass.h
  lib/gettext.h
  lib/hmac-md5.c
  lib/hmac-sha1.c
  lib/hmac.h
  lib/inet_ntop.c
  lib/inet_ntop.h
  lib/inet_pton.c
  lib/inet_pton.h
  lib/md2.c
  lib/md2.h
  lib/md4.c
  lib/md4.h
  lib/md5.c
  lib/md5.h
  lib/memmem.c
  lib/memmem.h
  lib/memmove.c
  lib/memxor.c
  lib/memxor.h
  lib/minmax.h
  lib/printf-args.c
  lib/printf-args.h
  lib/printf-parse.c
  lib/printf-parse.h
  lib/read-file.c
  lib/read-file.h
  lib/readline.c
  lib/readline.h
  lib/rijndael-alg-fst.c
  lib/rijndael-alg-fst.h
  lib/rijndael-api-fst.c
  lib/rijndael-api-fst.h
  lib/sha1.c
  lib/sha1.h
  lib/size_max.h
  lib/snprintf.c
  lib/snprintf.h
  lib/socket_.h
  lib/stdbool_.h
  lib/stdint_.h
  lib/strdup.c
  lib/strdup.h
  lib/vasnprintf.c
  lib/vasnprintf.h
  lib/xsize.h
  m4/absolute-header.m4
  m4/alloca.m4
  m4/arcfour.m4
  m4/arctwo.m4
  m4/arpa_inet_h.m4
  m4/codeset.m4
  m4/des.m4
  m4/eoverflow.m4
  m4/gc-arcfour.m4
  m4/gc-arctwo.m4
  m4/gc-des.m4
  m4/gc-hmac-md5.m4
  m4/gc-hmac-sha1.m4
  m4/gc-md2.m4
  m4/gc-md4.m4
  m4/gc-md5.m4
  m4/gc-pbkdf2-sha1.m4
  m4/gc-random.m4
  m4/gc-rijndael.m4
  m4/gc-sha1.m4
  m4/gc.m4
  m4/getaddrinfo.m4
  m4/getdelim.m4
  m4/getline.m4
  m4/getpass.m4
  m4/gettext.m4
  m4/glibc2.m4
  m4/glibc21.m4
  m4/hmac-md5.m4
  m4/hmac-sha1.m4
  m4/iconv.m4
  m4/inet_ntop.m4
  m4/inet_pton.m4
  m4/intdiv0.m4
  m4/intmax.m4
  m4/intmax_t.m4
  m4/inttypes-h.m4
  m4/inttypes-pri.m4
  m4/inttypes_h.m4
  m4/lcmessage.m4
  m4/lib-ld.m4
  m4/lib-link.m4
  m4/lib-prefix.m4
  m4/lock.m4
  m4/longdouble.m4
  m4/longlong.m4
  m4/md2.m4
  m4/md4.m4
  m4/md5.m4
  m4/memmem.m4
  m4/memmove.m4
  m4/memxor.m4
  m4/minmax.m4
  m4/netinet_in_h.m4
  m4/nls.m4
  m4/po.m4
  m4/printf-posix.m4
  m4/progtest.m4
  m4/read-file.m4
  m4/readline.m4
  m4/rijndael.m4
  m4/sha1.m4
  m4/signed.m4
  m4/size_max.m4
  m4/snprintf.m4
  m4/socklen.m4
  m4/sockpfaf.m4
  m4/stdbool.m4
  m4/stdint.m4
  m4/stdint_h.m4
  m4/strdup.m4
  m4/sys_socket_h.m4
  m4/uintmax_t.m4
  m4/ulonglong.m4
  m4/vasnprintf.m4
  m4/visibility.m4
  m4/wchar_t.m4
  m4/wint_t.m4
  m4/xsize.m4
])
