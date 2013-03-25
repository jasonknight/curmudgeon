bin_PROGRAMS = regex

regex_SOURCES = regex.c

regex_CFLAGS = $(MYSQL_CFLAGS) $(LIBPCRE_CFLAGS) -I$(top_srcdir)/src/curmudgeon

regex_LDADD = -lcurmudgeon $(MYSQL_LIBS) $(LIBPCRE_LIBS) -L$(top_srcdir)/src/curmudgeon