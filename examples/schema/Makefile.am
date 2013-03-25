bin_PROGRAMS = schema

schema_SOURCES = schema.c

schema_CFLAGS = $(MYSQL_CFLAGS) $(LIBPCRE_CFLAGS) -I$(top_srcdir)/src/curmudgeon -I$(top_srcdir)/src/jansson

schema_LDFLAGS = -lcurmudgeon -ljansson $(MYSQL_LIBS) $(LIBPCRE_LIBS) -L$(top_srcdir)/src/curmudgeon -L$(top_srcdir)/src/jansson