#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE(foo, 1.0)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.


# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strdup])

AM_PROG_LIBTOOL

PKG_CHECK_MODULES([LIBPCRE], [libpcre])

# MF: Get MySQL library and include locations
AC_ARG_WITH([mysql-include-path],
  [AS_HELP_STRING([--with-mysql-include-path],
    [location of the MySQL headers, defaults to /usr/include/mysql])],
  [MYSQL_CFLAGS="-I$withval"],
  [MYSQL_CFLAGS='-I/usr/include/mysql'])
AC_SUBST([MYSQL_CFLAGS])

AC_ARG_WITH([mysql-lib-path],
  [AS_HELP_STRING([--with-mysql-lib-path], [location of the MySQL libraries])],
  [MYSQL_LIBS="-L$withval -lmysqlclient"],
  [MYSQL_LIBS='-lmysqlclient'])
AC_SUBST([MYSQL_LIBS])


AC_CONFIG_FILES([Makefile                       \
                src/Makefile                    \
                src/jansson/Makefile            \
                src/curmudgeon/Makefile         \
                src/examples/Makefile           \
                src/examples/json/Makefile      \
                src/examples/options/Makefile   \
                src/examples/regex/Makefile     \
                src/examples/schema/Makefile    \
                ])
AC_OUTPUT
