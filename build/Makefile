#LDFLAGS=-static -static-libgcc
FLAGS=-I../src -I../src/curmudgeon -I../src/parson -g
CURMUDGE_FLAGS=-lpcre -Wno-incompatible-pointer-types-discards-qualifiers -g
MYSQL=-I/usr/include/mysql -DBIG_JOINS=1  -fno-strict-aliasing  -g -L/usr/lib/x86_64-linux-gnu -lmysqlclient
JSON=${SRC}/parson
JSON_OBJS=${JSON}/parson.o
SRC=../src
CC=clang
all:

options: curmudgeon_json.o 
	${CC} ${JSON_OBJS} curmudgeon_json.o ${SRC}/examples/options.c -o options.bin ${FLAGS} ${MYSQL} ${LDFLAGS} ${CURMUDGE_FLAGS}
schema: curmudgeon.o 
	${CC} ${JSON_OBJS} curmudgeon.o ${SRC}/examples/schema.c -o schema.bin ${FLAGS} ${MYSQL} ${LDFLAGS} ${CURMUDGE_FLAGS}
regex: curmudgeon.o 
	${CC} ${JSON_OBJS} curmudgeon.o ${SRC}/examples/regex.c -o regex.bin ${FLAGS} ${MYSQL} ${LDFLAGS} ${CURMUDGE_FLAGS}
example: curmudgeon.o 
	${CC} ${JSON_OBJS} curmudgeon.o curmudgeon_json.o ${SRC}/examples/main.c -o example.bin ${FLAGS} ${MYSQL} ${LDFLAGS} ${CURMUDGE_FLAGS}
curmudgeon.o: ${SRC}/curmudgeon/curmudgeon.c
	${CC}  ${JSON_OBJS} curmudgeon_json.o -c ${SRC}/curmudgeon/curmudgeon.c -o curmudgeon.o ${FLAGS} ${MYSQL} ${CURMUDGE_FLAGS}
curmudgeon_json.o: ${SRC}/curmudgeon/curmudgeon_json.c 
	${CC} -c ${JSON_OBJS} ${SRC}/curmudgeon/curmudgeon_json.c -o curmudgeon_json.o ${FLAGS} ${MYSQL} ${CURMUDGE_FLAGS}

#jansson:${JSON}/dump.c ${JSON}/error.c ${JSON}/hashtable.c ${JSON}/load.c ${JSON}/memory.c ${JSON}/pack_unpack.c ${JSON}/load.c ${JSON}/load.c 
#	${CC} -c ${JSON}/dump.c -o dump.o ${FLAGS}
#	${CC} -c ${JSON}/error.c -o error.o ${FLAGS}
#	${CC} -c ${JSON}/hashtable.c -o hashtable.o ${FLAGS}
#	${CC} -c ${JSON}/load.c -o load.o ${FLAGS}
#	${CC} -c ${JSON}/memory.c -o memory.o ${FLAGS}
#	${CC} -c ${JSON}/pack_unpack.c -o pack_unpack.o ${FLAGS}
#	${CC} -c ${JSON}/strbuffer.c -o strbuffer.o ${FLAGS}
#	${CC} -c ${JSON}/utf.c -o utf.o ${FLAGS}
#	${CC} -c ${JSON}/value.c -o value.o ${FLAGS}
#	${CC} -c ${JSON}/strconv.c -o strconv.o ${FLAGS}
clean:
	rm *.o
	rm *.bin
