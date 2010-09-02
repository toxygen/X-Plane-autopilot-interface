CC=gcc
APL=1
IBM=0
SDKDIR=SDK
#CFLAGS=-std=c99
CFLAGS=-std=c99 -Wall -pedantic
all: ui interface server client link
OBJECTS=*.o

clean:
	rm ${OBJECTS}
	rm interface.xpl
	rm client

client:	client.c
	$(CC) ${CFLAGS} client.c -o client

interface: interface.c
	$(CC) ${CFLAGS} -I${SDKDIR} -DAPL=${APL} -DIBM=${IBM} interface.c -o interface.o -flat_namespace -undefined dynamic_lookup -arch i386 -c

link: ui.o interface.o server.o
	$(CC) ${OBJECTS} -o interface.xpl -dynamiclib -flat_namespace -undefined dynamic_lookup -arch i386 

ui: ui.c
	$(CC) ${CFLAGS} -I${SDKDIR} -DAPL=${APL} -DIBM=${IBM} ui.c -o ui.o -flat_namespace -undefined dynamic_lookup -arch i386 -c

server: server.c
	$(CC) ${CFLAGS} -I${SDKDIR} -DAPL=${APL} -DIBM=${IBM} server.c -o server.o -flat_namespace -undefined dynamic_lookup -arch i386 -c
