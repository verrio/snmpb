  ############################################################################
  ## 
  ##  configure.ac  
  ##
  ##  SNMP++ v3.3
  ##  -----------------------------------------------
  ##  Copyright (c) 2001-2013 Jochen Katz, Frank Fock
  ##
  ##  This software is based on SNMP++2.6 from Hewlett Packard:
  ##  
  ##    Copyright (c) 1996
  ##    Hewlett-Packard Company
  ##  
  ##  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  ##  Permission to use, copy, modify, distribute andor sell this software 
  ##  andor its documentation is hereby granted without fee. User agrees 
  ##  to display the above copyright notice and this license notice in all 
  ##  copies of the software and any documentation of the software. User 
  ##  agrees to assume all liability for the use of the software; 
  ##  Hewlett-Packard and Jochen Katz make no representations about the 
  ##  suitability of this software for any purpose. It is provided 
  ##  "AS-IS" without warranty of any kind, either express or implied. User 
  ##  hereby grants a royalty-free license to any and all derivatives based
  ##  upon this software code base. 
  ##  
  ##########################################################################*
dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

dnl don't require higher autoconf version for win32 support
AC_PREREQ([2.64])
AC_INIT([snmp++], [3.3.10], [support@agentpp.com],[snmp++])
SNMP_PP_MAJOR_VERSION=3
SNMP_PP_MINOR_VERSION=3
SNMP_PP_MICRO_VERSION=10
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_SRCDIR([src/oid.cpp])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE([disable])

dnl
dnl Get cannonical host
dnl
AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED([OS_TYPE], "$host_os",
                   [Define to be the name of the operating system.])

dnl disable unnecessary compiler checks
AC_DEFUN([AC_PROG_F77], [:])
AC_DEFUN([AC_PROG_FC], [:])
AC_DEFUN([AC_PROG_OBJC], [:])
AC_DEFUN([AC_PROG_OBJCCPP], [:])
AC_DEFUN([AC_LIBTOOL_F77], [:])

dnl disable check for GNU compatible malloc - just a working malloc is enough
AC_DEFUN([AC_FUNC_MALLOC_IF], [:])
AC_DEFUN([AC_FUNC_MALLOC], [
  AC_CHECK_FUNCS([malloc realloc])
])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_GREP
AC_PROG_SED
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

dnl Check for extra flags to enable some kind of behavior
AC_CHECK_DECL([_AIX],[ac_cv_need_minus_d_linux_source_compat="yes"; ac_cv_need_minus_d_all_source="yes"])
AC_CHECK_DECL([__linux],[ac_cv_need_minus_d_gnu_source="yes"]) dnl probably required by kfreebsd, too?
AC_CHECK_DECL([__sun],[ac_cv_need_minus_d_extensions="yes"])

AS_IF([test "x$ac_cv_need_minus_d_linux_source_compat" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D_LINUX_SOURCE_COMPAT"])

AS_IF([test "x$ac_cv_need_minus_d_gnu_source" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"])

AS_IF([test "x$ac_cv_need_minus_d_all_source" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D_ALL_SOURCE"])

AS_IF([test "x$ac_cv_need_minus_d_extensions" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D__EXTENSIONS__"])

ACX_CHECK_XOPEN_SOURCE
AS_IF([test -n "$XOPEN_SOURCE_CPPFLAGS"], [CPPFLAGS="$CPPFLAGS $XOPEN_SOURCE_CPPFLAGS"])

dnl
dnl Process features.
dnl
dnl ACX_CHECK_WITH_LIB([ssl],[check])
dnl ACX_CHECK_WITH_LIB([tomcrypt],[check])
dnl ACX_CHECK_WITH_LIB([des],[check])
ACX_CHECK_WITH_LIB([ssl])
ACX_CHECK_WITH_LIB([tomcrypt])
ACX_CHECK_WITH_LIB([des])

AC_ARG_ENABLE(debug,
[  --disable-debug         disable support for debugging output],
ENABLE_DEBUG=$enableval, ENABLE_DEBUG=yes)
AC_ARG_ENABLE(namespace,
[  --enable-namespace      enable using of namespace (default: on if supported)
  --disable-namespace     disable using of namespace],
ENABLE_NAMESPACE=$enableval, ENABLE_NAMESPACE=check)
AC_ARG_ENABLE(logging,
[  --enable-logging        enable built-in logging (default: on)
  --disable-logging       disable built-in logging],
ENABLE_LOGGING=$enableval, ENABLE_LOGGING=yes)
AC_ARG_ENABLE(threads,
[  --disable-threads       disable thread support],
ENABLE_THREADS=$enableval, ENABLE_THREADS=yes)
AC_ARG_ENABLE(snmpv3,
[  --disable-snmpv3        disable SNMPv3 support],
ENABLE_SNMPV3=$enableval, ENABLE_SNMPV3=check)
AC_ARG_ENABLE(macaddress,
[  --enable-macaddress     enable support of class MacAddress (default: off)
  --disable-macaddress    disable support of class MacAddress],
ENABLE_MACADDRESS=$enableval, ENABLE_MACADDRESS=no)
AC_ARG_ENABLE(ipxaddress,
[  --enable-ipxaddress     enable support of class IpxAddress (default: off)
  --disable-ipxaddress    disable support of class IpxAddress],
ENABLE_IPXADDRESS=$enableval, ENABLE_IPXADDRESS=no)

AC_ARG_WITH([log-profile], [
  --with-log-profile[[=profile]]
                          uses a specific profile for logging
                          * off:      does not log anything
                          * quiet:    logs only errors and warnings
                          * std:      logs (all errors and warnings) and events
                                      up to severity 5 (default if profile
                                      omitted)
                          * events:   logs all errors, warnings and events
                          * verbose:  logs (all errors, warnings and events)
                                      and info up to severity 5
                          * full:     logs all errors, warnings, events and
                                      info messages
                          * debug:    logs (all errors, warnings, events and
                                      info messages) and debug messages up to
                                      severity 5
                          * schwafel: logs all with each severity
                          * original: uses the very interesting original log
                                      profile (default if omitted)
  --without-log-profile  same as --with-log-default=original],
[WITH_LOG_PROFILE="$withval"], [WITH_LOG_PROFILE="default"])

AS_IF([test "x${ENABLE_DEBUG}" = "xyes"],
      [AC_DEFINE([DEBUG], 1, [Define if you want the debug output support compiled in.])],
      [AC_LIB_APPENDTOVAR([CXXFLAGS], [-D_NDEBUG])]
)
AS_IF([test "x${ENABLE_MACADDRESS}" = "xyes"], [WITH_MACADDRESS=1], [WITH_MACADDRESS=0])
AS_IF([test "x${ENABLE_IPXADDRESS}" = "xyes"], [WITH_IPXADDRESS=1], [WITH_IPXADDRESS=0])

dnl Checks for optional dependencies
AS_IF([test "x${ENABLE_THREADS}" = "xyes"], [
    ACX_WIN32(
    [
        AC_DEFINE([ENABLE_THREADS], 1,
            [Define if you want the thread support compiled in.])
        ac_cv_enable_threads=yes
        WITH_THREADS=1
    ],
    [
        ACX_PTHREAD([
            AC_DEFINE([ENABLE_THREADS], 1,
                [Define if you want the thread support compiled in.])
            AC_DEFINE(HAVE_PTHREAD, 1,
                [Define if you have POSIX threads libraries and header files.])
            CC="$PTHREAD_CC"
            CXX="$PTHREAD_CXX"
            AS_IF([test -n "$PTHREAD_LIBS"], [LIBS="$PTHREAD_LIBS $LIBS"])
            AS_IF([test -n "$PTHREAD_CFLAGS"], [
              CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
              CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
            ])
            ac_cv_enable_threads=yes
            WITH_PTHREAD=1
            WITH_THREADS=1
        ],
        [])
    ])
], [WITH_THREADS=0
    WITH_PTHREAD=0]) # endif : --enable-threads

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h ctype.h errno.h float.h inttypes.h limits.h memory.h netdb.h netinet/in.h poll.h signal.h stdio.h stdlib.h string.h strings.h sys/param.h sys/select.h sys/socket.h sys/time.h sys/timeb.h sys/types.h sys/unistd.h time.h unistd.h])

if test "$ac_cv_header_stdc" = "yes"; then
	CNF_STDC_HEADERS=1
else
	CNF_STDC_HEADERS=0
fi

if test "$ac_cv_header_sys_types_h" = "yes"; then
	CNF_HAVE_SYS_TYPES_H=1
else
	CNF_HAVE_SYS_TYPES_H=0
fi
if test "$ac_cv_header_sys_param_h" = "yes"; then
	CNF_HAVE_SYS_PARAM_H=1
else
	CNF_HAVE_SYS_PARAM_H=0
fi
if test "$ac_cv_header_sys_stat_h" = "yes"; then
	CNF_HAVE_SYS_STAT_H=1
else
	CNF_HAVE_SYS_STAT_H=0
fi
if test "$ac_cv_header_stdlib_h" = "yes"; then
	CNF_HAVE_STDLIB_H=1
else
	CNF_HAVE_STDLIB_H=0
fi
if test "$ac_cv_header_unistd_h" = "yes"; then
	CNF_HAVE_UNISTD_H=1
else
	CNF_HAVE_UNISTD_H=0
fi
if test "$ac_cv_header_memory_h" = "yes"; then
	CNF_HAVE_MEMORY_H=1
else
	CNF_HAVE_MEMORY_H=0
fi
if test "$ac_cv_header_sys_unistd_h" = "yes"; then
	CNF_HAVE_SYS_UNISTD_H=1
else
	CNF_HAVE_SYS_UNISTD_H=0
fi
if test "$ac_cv_header_stdint_h" = "yes"; then
	CNF_HAVE_STDINT_H=1
else
	CNF_HAVE_STDINT_H=0
fi
if test "$ac_cv_header_poll_h" = "yes"; then
	CNF_HAVE_POLL_H=1
else
	CNF_HAVE_POLL_H=0
fi
if test "$ac_cv_header_sys_select_h" = "yes"; then
	CNF_HAVE_SYS_SELECT_H=1
else
	CNF_HAVE_SYS_SELECT_H=0
fi
if test "$ac_cv_header_string_h" = "yes"; then
	CNF_HAVE_STRING_H=1
else
	CNF_HAVE_STRING_H=0
fi
if test "$ac_cv_header_strings_h" = "yes"; then
	CNF_HAVE_STRINGS_H=1
else
	CNF_HAVE_STRINGS_H=0
fi
if test "$ac_cv_header_inttypes_h" = "yes"; then
	CNF_HAVE_INTTYPES_H=1
else
	CNF_HAVE_INTTYPES_H=0
fi
if test "$ac_cv_header_ctype_h" = "yes"; then
	CNF_HAVE_CTYPE_H=1
else
	CNF_HAVE_CTYPE_H=0
fi
if test "$ac_cv_header_signal_h" = "yes"; then
	CNF_HAVE_SIGNAL_H=1
else
	CNF_HAVE_SIGNAL_H=0
fi
if test "$ac_cv_header_errno_h" = "yes"; then
	CNF_HAVE_ERRNO_H=1
else
	CNF_HAVE_ERRNO_H=0
fi
if test "$ac_cv_header_time_h" = "yes"; then
	CNF_HAVE_TIME_H=1
else
	CNF_HAVE_TIME_H=0
fi
if test "$ac_cv_header_sys_time_h" = "yes"; then
	CNF_HAVE_SYS_TIME_H=1
else
	CNF_HAVE_SYS_TIME_H=0
fi
if test "$ac_cv_header_sys_timeb_h" = "yes"; then
	CNF_HAVE_SYS_TIMEB_H=1
else
	CNF_HAVE_SYS_TIMEB_H=0
fi
if test "$ac_cv_header_sys_socket_h" = "yes"; then
	CNF_HAVE_SYS_SOCKET_H=1
else
	CNF_HAVE_SYS_SOCKET_H=0
fi
if test "$ac_cv_header_netdb_h" = "yes"; then
	CNF_HAVE_NETDB_H=1
else
	CNF_HAVE_NETDB_H=0
fi
if test "$ac_cv_header_arpa_inet_h" = "yes"; then
	CNF_HAVE_ARPA_INET_H=1
else
	CNF_HAVE_ARPA_INET_H=0
fi
if test "$ac_cv_header_netinet_in_h" = "yes"; then
	CNF_HAVE_NETINET_IN_H=1
else
	CNF_HAVE_NETINET_IN_H=0
fi


dnl Checks for inet libraries:
AC_SEARCH_LIBS(gethostent, [nsl])
AC_SEARCH_LIBS(setsockopt, [socket net network])
AC_SEARCH_LIBS(connect, [inet])

AC_SEARCH_LIBS([clock_gettime],[rt posix4])

ACX_WIN32(
    AC_CHECK_HEADERS([io.h process.h winsock.h winsock2.h ws2tcpip.h wspiapi.h])
    AC_CHECK_LIB([wsock32],[main])
    AC_CHECK_LIB([ws2_32],[main]),
    []
)

if test "$ac_cv_header_io_h" = "yes"; then
	CNF_HAVE_IO_H=1
else
	CNF_HAVE_IO_H=0
fi
if test "$ac_cv_header_process_h" = "yes"; then
	CNF_HAVE_PROCESS_H=1
else
	CNF_HAVE_PROCESS_H=0
fi
if test "$ac_cv_header_winsock_h" = "yes"; then
	CNF_HAVE_WINSOCK_H=1
else
	CNF_HAVE_WINSOCK_H=0
fi
if test "$ac_cv_header_winsock2_h" = "yes"; then
	CNF_HAVE_WINSOCK2_H=1
else
	CNF_HAVE_WINSOCK2_H=0
fi
if test "$ac_cv_header_ws2tcpip_h" = "yes"; then
	CNF_HAVE_WS2TCPIP_H=1
else
	CNF_HAVE_WS2TCPIP_H=0
fi
if test "$ac_cv_header_wspiapi_h" = "yes"; then
	CNF_HAVE_WSPIAPI_H=1
else
	CNF_HAVE_WSPIAPI_H=0
fi




dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_UID_T

AC_CHECK_TYPES([struct sockaddr_in6],,,[
#if HAVE_WINSOCK2_H
#  include <winsock2.h>
#  if HAVE_WS2TCPIP_H
#    include <ws2tcpip.h>
#  endif
#  if HAVE_WSPIAPI_H
#    include <wspiapi.h>
#  endif
#else
#  if HAVE_ARPA_INET_H
#    include <arpa/inet.h>
#  endif
#  if HAVE_SYS_TYPES_H
#    include <sys/types.h>
#  endif
#  if HAVE_SYS_SOCKET_H
#    include <sys/socket.h>
#  endif
#endif
])

AC_CHECK_MEMBERS([struct sockaddr_storage.ss_family, struct sockaddr_storage.__ss_family], [], [], [
#if HAVE_WINSOCK2_H
#  include <winsock2.h>
#  if HAVE_WS2TCPIP_H
#    include <ws2tcpip.h>
#  endif
#  if HAVE_WSPIAPI_H
#    include <wspiapi.h>
#  endif
#elif HAVE_WINSOCK_H
#  include <winsock.h>
#  include <windows.h>
#else
#  if HAVE_ARPA_INET_H
#    include <arpa/inet.h>
#  endif
#  if HAVE_SYS_TYPES_H
#    include <sys/types.h>
#  endif
#  if HAVE_SYS_SOCKET_H
#    include <sys/socket.h>
#  endif
#endif
])

if test "$ac_cv_member_struct_sockaddr_storage_ss_family" = "yes"; then
	CNF_HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY=1
else
	CNF_HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY=0
fi
if test "$ac_cv_member_struct_sockaddr_storage___ss_family" = "yes"; then
	CNF_HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY=1
else
	CNF_HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY=0
fi


AC_LANG_PUSH([C++])
dnl Determine what socket length (socklen_t) data type is
AC_MSG_CHECKING([for type of socket length (socklen_t)])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>

int
main()
{
  (void)getsockopt (1, 1, 1, NULL, (socklen_t *)NULL);
  
  return 0;
}])],[AC_LANG_SOURCE([
  AC_MSG_RESULT(socklen_t *)
  SNMP_SOCKLEN_T=socklen_t],[
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>

int
main()
{
  (void)getsockopt (1, 1, 1, NULL, (size_t *)NULL);
  
  return 0;
}])])],[AC_LANG_SOURCE([
    AC_MSG_RESULT(size_t *)
    SNMP_SOCKLEN_T=size_t],[
    AC_COMPILE_IFELSE([
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>

int
main()
{
  (void)getsockopt (1, 1, 1, NULL, (int *)NULL);
  
  return 0;
}])],[AC_LANG_SOURCE([
      AC_MSG_RESULT(int *)
      SNMP_SOCKLEN_T=int])],[AC_LANG_SOURCE([
      AC_MSG_WARN(could not determine)
      SNMP_SOCKLEN_T="int"])])])])
AC_DEFINE_UNQUOTED(SNMP_SOCKLEN_T, $SNMP_SOCKLEN_T, [Determine what socket length (socklen_t) data type is])
AC_LANG_POP([C++])

AC_ARG_ENABLE(ipv6,
[  --disable-ipv6          disable IPv6 support],
ENABLE_IPV6=$enableval,
AS_IF([test "x$ac_cv_type_struct_sockaddr_in6" != "x"], [ENABLE_IPV6=yes], []))

AS_IF([test "x$ENABLE_IPV6" = "xyes"], [
    AS_IF([test x"$ac_cv_type_struct_sockaddr_in6" = x], [
        AC_MSG_FAILURE([IPv6 wanted but not available],[255])
    ]) # endif
    AC_DEFINE([SNMP_PP_IPv6], 1,
        [define SNMP_PP_IPv6 if you want to use IPv6])
    WITH_SNMP_PP_IPv6=1
],
[WITH_SNMP_PP_IPv6=0]) dnl endif : --enable-ipv6

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gethostbyaddr gethostbyaddr_r gethostbyname gethostbyname2 gethostbyname_r gethostname gettimeofday getpid _getpid htonl inet_aton inet_ntoa inet_pton inet_ntop isdigit localtime_r memset poll select socket strcasecmp stricmp strchr strerror strstr clock_gettime])

if test "$ac_cv_func_strcasecmp" = "yes"; then
	CNF_HAVE_STRCASECMP=1
else
	CNF_HAVE_STRCASECMP=0
fi
if test "$ac_cv_func_stricmp" = "yes"; then
	CNF_HAVE_STRICMP=1
else
	CNF_HAVE_STRICMP=0
fi
if test "$ac_cv_func_getpid" = "yes"; then
	CNF_HAVE_GETPID=1
else
	CNF_HAVE_GETPID=0
fi
if test "$ac_cv_func__getpid" = "yes"; then
	CNF_HAVE__GETPID=1
else
	CNF_HAVE__GETPID=0
fi

acx_have_getaddrinfo=no
AC_CHECK_FUNC(getaddrinfo, acx_have_getaddrinfo=yes)
AS_IF([test "x$acx_have_getaddrinfo" != "xyes"], [
  for lib in bsd socket inet; do
    AC_CHECK_LIB($lib, getaddrinfo, [
      AC_LIB_APPENDTOVAR([LIBS], [-l${lib}])
      acx_have_getaddrinfo=yes
      break
    ])
  done
])

AS_IF([test "x$acx_have_getaddrinfo" != "xyes"], [
  dnl check this separately if it produces different results on Win2k or WinXP
  AC_CHECK_DECLS([getaddrinfo],acx_have_getaddrinfo=yes,,[AC_LANG_SOURCE([
#if HAVE_WINSOCK2_H
#  include <winsock2.h>
#  if HAVE_WS2TCPIP_H
#    // Fake windows XP for getaddrinfo if IPv6
#    if HAVE_STRUCT_SOCKADDR_IN6
#      define _WIN32_WINNT 0x0501
#    endif
#    include <ws2tcpip.h>
#  endif
#  if HAVE_WSPIAPI_H
#    include <wspiapi.>
#  endif
#else
#  include <netdb.h>
#endif
  ])])
])

AS_IF([test "x$acx_have_getaddrinfo" = "xyes"], [
  AC_DEFINE([HAVE_GETADDRINFO], 1, [Define if getaddrinfo is there])
])

AX_CXX_NAMESPACES
AX_CXX_NAMESPACE_STD
AX_CXX_HAVE_STL
AX_CXX_HAVE_STD
AX_CXX_TEMPLATES
AX_CXX_HEADER_STDCXX_98
AX_CXX_HEADER_STDCXX_TR1
# Check for C++0x features
AC_CXX_COMPILE_STDCXX_0X
AX_CXX_HEADER_STDCXX_0X

if test "$ax_cv_cxx_have_std_namespace" = "yes"; then
	CNF_HAVE_NAMESPACE_STD=1
else
	CNF_HAVE_NAMESPACE_STD=0
fi

AS_IF(
  [test "x${ENABLE_NAMESPACE}" = "xyes"],
  [
    AS_IF([test "x$ax_cv_cxx_namespaces" = "xyes"],
          [
            WITH_NAMESPACE=1
            AC_MSG_NOTICE([enable using of namespace Snmp_pp])
          ],
          [AC_MSG_FAILURE([C++ compiler does not support namespaces])])
  ],
  [test "x${ENABLE_NAMESPACE}" = "xcheck"],
  [
    AS_IF([test "x$ax_cv_cxx_namespaces" = "xyes"],
          [
            WITH_NAMESPACE=1
            AC_MSG_NOTICE([enable using of namespace Snmp_pp])
          ],
          [
            WITH_NAMESPACE=0
            AC_MSG_NOTICE([disable using of namespace Snmp_pp])
          ])
  ],
  [
    WITH_NAMESPACE=0
    AC_MSG_NOTICE([disable using of namespace Snmp_pp])
  ]
)

WITH_LOG_PROFILES=0
AS_IF([test "x${ENABLE_LOGGING}" != "xno"],
      [
        WITH_LOGGING=1
        AS_IF([test "x$ax_cv_cxx_have_stl" = "xyes" -a "x$WITH_LOG_PROFILE" != "xno"], [
          AC_MSG_CHECKING([which log-profile will be used])
          AS_CASE(
            [$WITH_LOG_PROFILE],
            [[[Oo][Ff][Ff]]], [DEFAULT_LOG_PROFILE="off"],
            [[[Qq][Uu][Ii][Ee][Tt]]], [DEFAULT_LOG_PROFILE="quiet"],
            [[[Yy][Ee][Ss]|[Ss][Tt][Dd]]], [DEFAULT_LOG_PROFILE="std"],
            [[[Ee][Vv][Ee][Nn][Tt][Ss]]], [DEFAULT_LOG_PROFILE="events"],
            [[[Vv][Ee][Rr][Bb][Oo][Ss][Ee]]], [DEFAULT_LOG_PROFILE="verbose"],
            [[[Ff][Uu][Ll][Ll]]], [DEFAULT_LOG_PROFILE="full"],
            [[[De][Ee][Bb][Uu][Gg]]], [DEFAULT_LOG_PROFILE="debug"],
            [[[Ss][Cc][Hh][Ww][Aa][Ff][Ee][Ll]]], [DEFAULT_LOG_PROFILE="schwafel"],
            [[[Oo][Rr][Ii][Gg][Ii][Nn][Aa][Ll]]], [DEFAULT_LOG_PROFILE="original"],
            [[[Dd][Ee][Ff][Aa][Uu][Ll][Tt]]], [DEFAULT_LOG_PROFILE="original"],
            [
              AC_MSG_WARN([Invalid log profile "$WITH_LOG_PROFILE" - using "original"])
              DEFAULT_LOG_PROFILE="original"
            ]
          )
          AC_DEFINE_UNQUOTED([DEFAULT_LOG_PROFILE], "$DEFAULT_LOG_PROFILE", [Defines default log profile to use])
          AC_MSG_RESULT("$DEFAULT_LOG_PROFILE")
          WITH_LOG_PROFILES=1
        ], [test "x$DEFAULT_LOG_PROFILE" != "xdefault" ], [
          AC_MSG_FAILURE(log profiles require STL support)
        ])
      ],
      [WITH_LOGGING=0]
)

LINKFLAGS=""
CLIBFLAGS=""

dnl Now actually check for -lssl
WITH_SSL=0
ACX_CHECK_LIB_FLAGS([ssl], [crypto,dl,crypto dl], [SSL_library_init()], [AC_LANG_SOURCE([
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/des.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
  ])], [openssl],
  [
    WITH_LIBSSL=1
    AC_LIB_APPENDTOVAR([LINKFLAGS], ${LIBS_SSL})
    AC_LIB_APPENDTOVAR([CLIBFLAGS], ${INC_SSL})
    AC_MSG_NOTICE([compiling in support for SSL via OpenSSL (LIBS="$LIBS_SSL")])
  ],
  [WITH_LIBSSL=0]
)

WITH_LIBTOMCRYPT=0
dnl --with-tomcrypt: check if it's no
ACX_CHECK_LIB_FLAGS([tomcrypt], [], [find_cipher()], [AC_LANG_SOURCE([
#include <tomcrypt_cipher.h>
  ])], ,
  [
    WITH_LIBTOMCRYPT=1
    AC_LIB_APPENDTOVAR([LINKFLAGS], ${LIBS_TOMCRYPT})
    AC_LIB_APPENDTOVAR([CLIBFLAGS], ${INC_TOMCRYPT})
    AC_MSG_NOTICE([compiling in support for libtomcrypt (LIBS="$LIBS_TOMCRYPT}")])
  ],
  [WITH_LIBTOMCRYPT=0]
)

WITH_LIBDES=0
ACX_CHECK_LIB_FLAGS([des], [], [AC_LANG_SOURCE([
des_key_schedule ks;
des_ecb_encrypt((C_Block *)0,(C_Block *)0,ks,DES_ENCRYPT)])], [AC_LANG_SOURCE([
#include <des.h>
  ])], ,
  [AC_LANG_SOURCE([
    WITH_LIBDES=1
    AC_LIB_APPENDTOVAR([LINKFLAGS], ${LIBS_DES})
    AC_LIB_APPENDTOVAR([CLIBFLAGS], ${INC_DES})
    AC_MSG_NOTICE([compiling in support for libdes (LIBS="$LIBS_DES}")using des.h])
  ])],
  [
    dnl some packaging systems package libdes.h instead of des.h
    dnl (to avoid conflicts with whatsoever)
    unset acx_cv_found_libdes
    ACX_CHECK_LIB_FLAGS([des], [], [AC_LANG_SOURCE([
    des_key_schedule ks;
    des_ecb_encrypt((C_Block *)0,(C_Block *)0,ks,DES_ENCRYPT)])], [AC_LANG_SOURCE([
#include <libdes.h>
      ])], ,
      [
        WITH_LIBDES=1
        AC_DEFINE([HAVE_LIBDES_H], 1, [Define when libdes.h shall be favored over des.h])
        AC_LIB_APPENDTOVAR([LINKFLAGS], ${LIBS_DES})
        AC_LIB_APPENDTOVAR([CLIBFLAGS], ${INC_DES})
        AC_MSG_NOTICE([compiling in support for libdes (LIBS="$LIBS_DES}") using libdes.h])
      ],
      [WITH_LIBDES=0]
    )
  ]
)

AC_MSG_CHECKING([snmp v3 status])
_SNMPv3=0
AS_IF([test "x${ENABLE_SNMPV3}" = "xyes"], [
  AC_MSG_RESULT([required])
  AS_IF([test "x$WITH_LIBSSL" = "x0" -a "x$WITH_LIBTOMCRYPT" = "x0" -a "x$WITH_LIBDES" = "x0"], [
    AC_MSG_FAILURE([cryptographic library required for SNMPv3, but neither OpenSSL nor TomCrypt nor DES is available.],[255])
  ]) dnl endif : have a crypto library
  AC_DEFINE([_SNMPv3], 1, [define _SNMPv3 if you want to use SNMPv3])
  _SNMPv3=1
], [test "x${ENABLE_SNMPV3}" = "xcheck"], [
  AS_IF([test "x$WITH_LIBSSL" != "x0"], [
    AC_MSG_RESULT([supported (openssl)])
    AC_DEFINE([_SNMPv3], 1, [define _SNMPv3 if you want to use SNMPv3])
    _SNMPv3=1
  ], [test "x$WITH_LIBTOMCRYPT" != "x0"], [
    AC_MSG_RESULT([supported (tomcrypt)])
    AC_DEFINE([_SNMPv3], 1, [define _SNMPv3 if you want to use SNMPv3])
    _SNMPv3=1
  ], [test "x$WITH_LIBDES" != "x0"], [gettid
    AC_MSG_RESULT([supported (libdes)])
    AC_DEFINE([_SNMPv3], 1, [define _SNMPv3 if you want to use SNMPv3])
    _SNMPv3=1
  ], [
    AC_MSG_RESULT([not supported])
    AC_DEFINE([_NO_SNMPv3], 1, [define _NO_SNMPv3 if you do not want to use SNMPv3])
    _NO_SNMPv3=1
  ])
], [
  AC_MSG_RESULT([disabled])
  AC_DEFINE([_NO_SNMPv3], 1, [define _NO_SNMPv3 if you do not want to use SNMPv3])
  _NO_SNMPv3=1
]) dnl endif : --enable-snmpv3



if test "$ax_cv_cxx_stdcxx_98" = yes; then
	CNF_STDCXX_98_HEADERS=1
else
	CNF_STDCXX_98_HEADERS=0
fi


LT_INIT
AC_PROG_LIBTOOL

AC_SUBST(SNMP_PP_MAJOR_VERSION)
AC_SUBST(SNMP_PP_MINOR_VERSION)
AC_SUBST(SNMP_PP_MICRO_VERSION)

AC_SUBST(LINKFLAGS)
AC_SUBST(CLIBFLAGS)

AC_SUBST(_SNMPv3)
AC_SUBST(WITH_THREADS)
AC_SUBST(WITH_SNMP_PP_IPv6)
AC_SUBST(WITH_LIBSSL)
AC_SUBST(WITH_LIBTOMCRYPT)
AC_SUBST(WITH_LIBDES)
AC_SUBST(WITH_LOGGING)
AC_SUBST(WITH_NAMESPACE)
AC_SUBST(WITH_PTHREAD)
AC_SUBST(WITH_MODULES)
AC_SUBST(WITH_MACADDRESS)
AC_SUBST(WITH_IPXADDRESS)
AC_SUBST(WITH_LOG_PROFILES)

AC_SUBST(CNF_STDC_HEADERS)
AC_SUBST(CNF_HAVE_SYS_TYPES_H)
AC_SUBST(CNF_HAVE_SYS_STAT_H)
AC_SUBST(CNF_HAVE_STDLIB_H)
AC_SUBST(CNF_HAVE_UNISTD_H)
AC_SUBST(CNF_HAVE_SYS_UNISTD_H)
AC_SUBST(CNF_HAVE_STDINT_H)
AC_SUBST(CNF_HAVE_POLL_H)
AC_SUBST(CNF_HAVE_SYS_SELECT_H)
AC_SUBST(CNF_HAVE_STRING_H)
AC_SUBST(CNF_HAVE_STRINGS_H)
AC_SUBST(CNF_HAVE_INTTYPES_H)
AC_SUBST(CNF_HAVE_CTYPE_H)
AC_SUBST(CNF_HAVE_SIGNAL_H)
AC_SUBST(CNF_HAVE_ERRNO_H)
AC_SUBST(CNF_HAVE_TIME_H)
AC_SUBST(CNF_HAVE_NETDB_H)
AC_SUBST(CNF_HAVE_MEMORY_H)
AC_SUBST(CNF_HAVE_SYS_TIME_H)
AC_SUBST(CNF_HAVE_SYS_TIMEB_H)
AC_SUBST(CNF_HAVE_SYS_SOCKET_H)
AC_SUBST(CNF_HAVE_ARPA_INET_H)
AC_SUBST(CNF_HAVE_NETINET_IN_H)
AC_SUBST(CNF_HAVE_IO_H)
AC_SUBST(CNF_HAVE_PROCESS_H)
AC_SUBST(CNF_HAVE_WINSOCK_H)
AC_SUBST(CNF_HAVE_WINSOCK2_H)
AC_SUBST(CNF_HAVE_WS2TCPIP_H)
AC_SUBST(CNF_HAVE_WSPIAPI_H)
AC_SUBST(CNF_STDCXX_98_HEADERS)
AC_SUBST(CNF_HAVE_STRCASECMP)
AC_SUBST(CNF_HAVE_STRICMP)
AC_SUBST(CNF_HAVE_GETPID)
AC_SUBST(CNF_HAVE__GETPID)
AC_SUBST(CNF_HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY)
AC_SUBST(CNF_HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY)
AC_SUBST(CNF_HAVE_NAMESPACE_STD)
AC_SUBST(CNF_HAVE_SYS_PARAM_H)

AC_CONFIG_FILES([Makefile include/Makefile include/snmp_pp/Makefile consoleExamples/Makefile src/Makefile])
AC_OUTPUT(snmp++-3.pc include/snmp_pp/config_snmp_pp.h libsnmp.h)
