bin_PROGRAMS = json

json_SOURCES = json.c

json_CFLAGS = $(MYSQL_CFLAGS) $(LIBPCRE_CFLAGS) -I$(top_srcdir)/src/curmudgeon -I$(top_srcdir)/src/jansson

json_LDFLAGS = -lcurmudgeon -ljansson $(MYSQL_LIBS) $(LIBPCRE_LIBS) -L$(top_srcdir)/src/curmudgeon -L$(top_srcdir)/src/jansson