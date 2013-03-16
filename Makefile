#LDFLAGS=-static -static-libgcc
FLAGS=-Isrc/ -Isrc/jansson/ -L/usr/lib/x86_64-linux-gnu -lpcre  
MYSQL=-I/usr/include/mysql/ `mysql_config --cflags --libs`
JSON=src/jansson/
JSON_OBJS=build/dump.o build/error.o build/hashtable.o build/load.o build/memory.o build/pack_unpack.o build/strbuffer.o build/utf.o build/value.o build/strconv.o
all:

regex: curmudgeon.o 
	clang ${JSON_OBJS} build/curmudgeon.o src/examples/regex.c -o build/regex.bin ${FLAGS} ${MYSQL} ${LDFLAGS}

example: curmudgeon.o 
	clang ${JSON_OBJS} build/curmudgeon.o src/examples/main.c -o build/example.bin ${FLAGS} ${MYSQL} ${LDFLAGS}
curmudgeon.o: jansson.o
	clang -c ${JSON_OBJS} src/curmudgeon.c -o build/curmudgeon.o ${FLAGS} ${MYSQL}
jansson.o:
	clang -c ${JSON}dump.c -o build/dump.o ${FLAGS}
	clang -c ${JSON}error.c -o build/error.o ${FLAGS}
	clang -c ${JSON}hashtable.c -o build/hashtable.o ${FLAGS}
	clang -c ${JSON}load.c -o build/load.o ${FLAGS}
	clang -c ${JSON}memory.c -o build/memory.o ${FLAGS}
	clang -c ${JSON}pack_unpack.c -o build/pack_unpack.o ${FLAGS}
	clang -c ${JSON}strbuffer.c -o build/strbuffer.o ${FLAGS}
	clang -c ${JSON}utf.c -o build/utf.o ${FLAGS}
	clang -c ${JSON}value.c -o build/value.o ${FLAGS}
	clang -c ${JSON}strconv.c -o build/strconv.o ${FLAGS}

