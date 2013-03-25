bin_PROGRAMS = options

options_SOURCES = options.c

options_CFLAGS = $(MYSQL_CFLAGS) $(LIBPCRE_CFLAGS) -I$(top_srcdir)/src/curmudgeon -I$(top_srcdir)/src/jansson

options_LDFLAGS = -lcurmudgeon -ljansson $(MYSQL_LIBS) $(LIBPCRE_LIBS) -L$(top_srcdir)/src/curmudgeon -L$(top_srcdir)/src/jansson