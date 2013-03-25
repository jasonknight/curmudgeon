INCLUDES = -I$(top_srcdir)/src/curmudgeon -I$(top_srcdir)/src/jansson

bin_PROGRAMS = schema

schema_SOURCES = schema.c

AM_CFLAGS = $(MYSQL_CFLAGS) $(LIBPCRE_CFLAGS)
schema_LDADD = -lcurmudgeon -ljansson $(MYSQL_LIBS) $(LIBPCRE_LIBS) -L$(top_srcdir)/src/curmudgeon -L$(top_srcdir)/src/jansson