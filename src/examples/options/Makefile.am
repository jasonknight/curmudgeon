INCLUDES = -I$(top_srcdir)/src/curmudgeon -I$(top_srcdir)/src/jansson

bin_PROGRAMS = options

options_SOURCES = options.c

AM_CFLAGS = $(MYSQL_CFLAGS) $(LIBPCRE_CFLAGS)
options_LDADD = -lcurmudgeon -ljansson $(MYSQL_LIBS) $(LIBPCRE_LIBS) -L$(top_srcdir)/src/curmudgeon -L$(top_srcdir)/src/jansson