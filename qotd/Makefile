#CC = 		/opt/SUNWspro/sunstudio12.1/bin/cc
CC = 		/usr/bin/gcc
LD =		/usr/ccs/bin/ld
STUDIO_FLAGS =	-D_KERNEL -m64 -xmodel=kernel
GCC_FLAGS =	-D_KERNEL -m64 -mcmodel=kernel -mno-red-zone -ffreestanding \
		-nodefaultlibs

#CFLAGS =	$(STUDIO_FLAGS)
CFLAGS =	$(GCC_FLAGS)

DEBUG_FLAGS =	-DDEBUG

all: qotd_1 qotd_2

qotd_1.o:
	$(CC) $(CFLAGS) -c -o qotd_1.o qotd_1.c

qotd_2.o:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c -o qotd_2.o qotd_2.c

qotd_1: qotd_1.o
	$(LD) -r -o qotd_1 qotd_1.o

qotd_2: qotd_2.o
	$(LD) -r -o qotd_2 qotd_2.o

clean:
	-rm -f qotd_?.o qotd_?

install:
	sudo cp qotd_?.conf /usr/kernel/drv
	sudo cp qotd_? /usr/kernel/drv/amd64
