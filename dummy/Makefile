#CC = 		/opt/SUNWspro/sunstudio12.1/bin/cc
CC = 		/usr/bin/gcc
LD =		/usr/ccs/bin/ld
STUDIO_FLAGS =	-D_KERNEL -m64 -xmodel=kernel
GCC_FLAGS =	-D_KERNEL -m64 -mcmodel=kernel -mno-red-zone -ffreestanding \
		-nodefaultlibs

#CFLAGS =	$(STUDIO_FLAGS)
CFLAGS =	$(GCC_FLAGS)

all: dummy

dummy.o:
	$(CC) $(CFLAGS) -c -o dummy.o dummy.c

dummy: dummy.o
	$(LD) -r -o dummy dummy.o

clean:
	-rm -f dummy.o dummy

install:
	sudo cp dummy.conf /usr/kernel/drv
	cp dummy /tmp
	sudo ln -s /tmp/dummy /usr/kernel/drv/amd64/dummy
