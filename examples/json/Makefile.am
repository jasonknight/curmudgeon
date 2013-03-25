bin_PROGRAMS = json

json_SOURCES = json.c

json_CFLAGS = $(MYSQL_CFLAGS) $(LIBPCRE_CFLAGS) -I$(top_srcdir)/src/curmudgeon

json_LDFLAGS = -lcurmudgeon $(MYSQL_LIBS) $(LIBPCRE_LIBS) -L$(top_srcdir)/src/curmudgeon