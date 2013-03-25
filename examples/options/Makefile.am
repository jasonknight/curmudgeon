bin_PROGRAMS = options

options_SOURCES = options.c

options_CFLAGS = $(MYSQL_CFLAGS) $(LIBPCRE_CFLAGS) -I$(top_srcdir)/src/curmudgeon

options_LDFLAGS = -lcurmudgeon $(MYSQL_LIBS) $(LIBPCRE_LIBS) -L$(top_srcdir)/src/curmudgeon