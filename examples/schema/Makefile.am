bin_PROGRAMS = schema

schema_SOURCES = schema.c

schema_CFLAGS = $(MYSQL_CFLAGS) $(LIBPCRE_CFLAGS) -I$(top_srcdir)/src/curmudgeon

schema_LDFLAGS = -lcurmudgeon $(MYSQL_LIBS) $(LIBPCRE_LIBS) -L$(top_srcdir)/src/curmudgeon