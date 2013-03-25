lib_LTLIBRARIES = libcurmudgeon.la

libcurmudgeon_la_SOURCES = curmudgeon_json.c curmudgeon.c

libcurmudgeon_la_CFLAGS = $(MYSQL_CFLAGS) $(LIBPCRE_CFLAGS) -I$(top_srcdir)/src/parson

libcurmudgeon_la_LIBADD = -lparson -L$(top_srcdir)/src/parson