APL=1
IBM=0
SDKDIR=SDK
#CFLAGS=-std=c99
CFLAGS=-std=c99 -Wall -pedantic
all: ui interface server link

clean:
	rm *.o

interface: interface.c
	gcc ${CFLAGS} -I${SDKDIR} -DAPL=${APL} -DIBM=${IBM} interface.c -o interface.o -flat_namespace -undefined dynamic_lookup -arch i386 -c

link: ui.o interface.o server.o
	gcc interface.o ui.o server.o -o interface.xpl -dynamiclib -flat_namespace -undefined dynamic_lookup -arch i386 


ui: ui.c
	gcc ${CFLAGS} -I${SDKDIR} -DAPL=${APL} -DIBM=${IBM} ui.c -o ui.o -flat_namespace -undefined dynamic_lookup -arch i386 -c

server: server.c
	gcc ${CFLAGS} -I${SDKDIR} -DAPL=${APL} -DIBM=${IBM} server.c -o server.o -flat_namespace -undefined dynamic_lookup -arch i386 -c
