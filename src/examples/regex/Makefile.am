INCLUDES = -I$(top_srcdir)/src/curmudgeon -I$(top_srcdir)/src/jansson

bin_PROGRAMS = regex

regex_SOURCES = regex.c

AM_CFLAGS = $(MYSQL_CFLAGS) $(LIBPCRE_CFLAGS)
regex_LDADD = -lcurmudgeon -ljansson $(MYSQL_LIBS) $(LIBPCRE_LIBS) -L$(top_srcdir)/src/curmudgeon -L$(top_srcdir)/src/jansson