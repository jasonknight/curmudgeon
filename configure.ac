#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([CURMUDGEON], [0.0.1], [office@red-e.eu])
AM_INIT_AUTOMAKE(curmudgeon, 0.0.1)
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
AM_PROG_LEX

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


# MF: Get libtidy locations
AC_ARG_WITH([tidy-include-path],
  [AS_HELP_STRING([--with-tidy-include-path],
    [location of the tidy headers, defaults to /usr/include/tidy])],
  [TIDY_CFLAGS="-I$withval"],
  [TIDY_CFLAGS='-I/usr/include/tidy'])
AC_SUBST([TIDY_CFLAGS])

AC_ARG_WITH([tidy-lib-path],
  [AS_HELP_STRING([--with-tidy-lib-path], [location of the tidy libraries])],
  [TIDY_LIBS="-L$withval -ltidy"],
  [TIDY_LIBS='-ltidy'])
AC_SUBST([TIDY_LIBS])


# DOXYGEN STUFF

AC_CHECK_PROGS([DOXYGEN], [doxygen])

if test -z "$DOXYGEN";
   then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi

AM_CONDITIONAL([HAVE_DOXYGEN], 
[test -n "$DOXYGEN"])AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doc/Doxyfile])])


prefix=/tmp/curmudgeon


AC_CONFIG_FILES([Makefile                         \
                src/Makefile                      \
                src/parson/Makefile               \
                src/haml/Makefile                 \
                src/sass/Makefile                 \
                src/curmudgeon/Makefile           \
                doc/Makefile                      \
                examples/Makefile                 \
                examples/json/Makefile            \
                examples/tidy/Makefile            \
                examples/haml/Makefile            \
                examples/sass/Makefile            \
                examples/options/Makefile         \
                examples/regex/Makefile           \
                examples/schema/Makefile          \
                ])
                
                

CC=clang
AC_OUTPUT
