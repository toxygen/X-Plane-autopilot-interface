APL=1
IBM=0
SDKDIR=SDK

all: ui interface link

interface: interface.c
	gcc -I${SDKDIR} -DAPL=${APL} -DIBM=${IBM} interface.c -o interface.o -flat_namespace -undefined dynamic_lookup -arch i386 -c

link: ui interface
	gcc interface.o ui.o -o interface.xpl -dynamiclib -flat_namespace -undefined dynamic_lookup -arch i386 


ui: ui.c
	gcc -I${SDKDIR} -DAPL=${APL} -DIBM=${IBM} ui.c -o ui.o -flat_namespace -undefined dynamic_lookup -arch i386 -c
