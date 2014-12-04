dnl
dnl Copyright 2008, 2009, 2010 Free Software Foundation, Inc.
dnl Copyright 2014 Range Networks, Inc.
dnl
dnl This software is distributed under the terms of the GNU Public License.
dnl See the COPYING file in the main directory for details.
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.
dnl

AC_PREREQ(2.57)
AC_INIT(smqueue,5.0-master)

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([-Wno-portability])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([config])
AC_CONFIG_SRCDIR([smqueue/smqueue.cpp])

AM_PROG_AS
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PATH_PROG([RM_PROG], [rm])

dnl AC_LIBTOOL_WIN32_DLL
AC_ENABLE_SHARED	dnl do build shared libraries
AC_DISABLE_STATIC	dnl don't build static libraries
AC_PROG_LIBTOOL

dnl Checks for header files.
AC_HEADER_STDC
dnl This is required for GnuRadio includes to understand endianess correctly:
AC_CHECK_HEADERS([byteswap.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_C_BIGENDIAN

dnl Check for libzmq
if test ! -r "/usr/include/zmq.h" -a ! -r "/usr/local/include/zmq.h"; then
    AC_MSG_ERROR([/usr/local/include/zmq.h not found. Install the libzmq3-dev package manually or run $ sudo ./NodeManager/install_libzmq.sh])
fi
if test ! -r "/usr/include/zmq.hpp" -a ! -r "/usr/local/include/zmq.hpp"; then
    AC_MSG_ERROR([/usr/local/include/zmq.hpp not found. Install the libzmq3-dev package manually or run $ sudo ./NodeManager/install_libzmq.sh])
fi
AC_CHECK_LIB(zmq, zmq_init, ,[AC_MSG_ERROR([Cannot link with -lzmq. Install the libzmq3-dev package manually or run $ sudo ./NodeManager/install_libzmq.sh])])
AC_MSG_CHECKING([whether libzmq installation works])
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <zmq.h>],
                                [zmq_init(1);])
               ],
               [AC_MSG_RESULT([yes])],
               [AC_MSG_ERROR([no. Install the libzmq3-dev package manually or run $ sudo ./NodeManager/install_libzmq.sh])])

# Defines OSIP_CFLAGS, OSIP_INCLUDEDIR, and OSIP_LIBS
PKG_CHECK_MODULES(OSIP, libosip2)

# Check for glibc-specific network functions
AC_CHECK_FUNC(gethostbyname_r, [AC_DEFINE(HAVE_GETHOSTBYNAME_R, 1, Define if libc implements gethostbyname_r)])
AC_CHECK_FUNC(gethostbyname2_r, [AC_DEFINE(HAVE_GETHOSTBYNAME2_R, 1, Define if libc implements gethostbyname2_r)])

dnl Output files
AC_CONFIG_FILES([\
    Makefile \
    config/Makefile \
    doc/Makefile \
    CommonLibs/Makefile \
    Globals/Makefile \
    GSM/Makefile \
    SMS/Makefile \
    NodeManager/Makefile \
    smqueue/Makefile \
    testing/Makefile \
])

AC_OUTPUT
