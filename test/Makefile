# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as
GCCDIR = /home/f85/kearns/nachos_compiler/bin/
#LDFLAGS = -T script -n -nostartfiles
LDFLAGS = -T nachos.ld -n -nostartfiles
ASFLAGS =
CPPFLAGS = $(INCDIR) -DCHANGED


# if you aren't cross-compiling:
#GCCDIR =
#LDFLAGS = -N -T 0
#ASFLAGS =
#CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)decstation-ultrix-gcc
AS = $(GCCDIR)decstation-ultrix-as
LD = $(GCCDIR)decstation-ultrix-ld

CPP = /usr/bin/cpp -E
INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR) -DCHANGED -nostartfiles

all: halt shell matmult sort fstest1 cp share1 share2 sharekid1 sharekid2 atmfile atmfileB atmfileC atmfileD makedir vmtorture Xkid qmatmult qsort

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c
shell: shell.o start.o
	$(LD) $(LDFLAGS) start.o shell.o -o shell.coff
	../bin/coff2noff shell.coff shell

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

fstest1.o: fstest1.c
	$(CC) $(CFLAGS) -c fstest1.c
fstest1: fstest1.o start.o
	$(LD) $(LDFLAGS) start.o fstest1.o -o fstest1.coff
	../bin/coff2noff fstest1.coff fstest1

cp.o: cp.c
	$(CC) $(CFLAGS) -c cp.c
cp: cp.o start.o
	$(LD) $(LDFLAGS) start.o cp.o -o cp.coff
	../bin/coff2noff cp.coff cp

share1.o: share1.c
	$(CC) $(CFLAGS) -c share1.c
share1: share1.o start.o
	$(LD) $(LDFLAGS) start.o share1.o -o share1.coff
	../bin/coff2noff share1.coff share1

share2.o: share2.c
	$(CC) $(CFLAGS) -c share2.c
share2: share2.o start.o
	$(LD) $(LDFLAGS) start.o share2.o -o share2.coff
	../bin/coff2noff share2.coff share2

sharekid1.o: sharekid1.c
	$(CC) $(CFLAGS) -c sharekid1.c
sharekid1: sharekid1.o start.o
	$(LD) $(LDFLAGS) start.o sharekid1.o -o sharekid1.coff
	../bin/coff2noff sharekid1.coff sharekid1

sharekid2.o: sharekid2.c
	$(CC) $(CFLAGS) -c sharekid2.c
sharekid2: sharekid2.o start.o
	$(LD) $(LDFLAGS) start.o sharekid2.o -o sharekid2.coff
	../bin/coff2noff sharekid2.coff sharekid2

atmfile.o: atmfile.c
	$(CC) $(CFLAGS) -c atmfile.c
atmfile: atmfile.o start.o
	$(LD) $(LDFLAGS) start.o atmfile.o -o atmfile.coff
	../bin/coff2noff atmfile.coff atmfile

atmfileB.o: atmfileB.c
	$(CC) $(CFLAGS) -c atmfileB.c
atmfileB: atmfileB.o start.o
	$(LD) $(LDFLAGS) start.o atmfileB.o -o atmfileB.coff
	../bin/coff2noff atmfileB.coff atmfileB

atmfileC.o: atmfileC.c
	$(CC) $(CFLAGS) -c atmfileC.c
atmfileC: atmfileC.o start.o
	$(LD) $(LDFLAGS) start.o atmfileC.o -o atmfileC.coff
	../bin/coff2noff atmfileC.coff atmfileC

atmfileD.o: atmfileD.c
	$(CC) $(CFLAGS) -c atmfileD.c
atmfileD: atmfileD.o start.o
	$(LD) $(LDFLAGS) start.o atmfileD.o -o atmfileD.coff
	../bin/coff2noff atmfileD.coff atmfileD

makedir.o: makedir.c
	$(CC) $(CFLAGS) -c makedir.c
makedir: makedir.o start.o
	$(LD) $(LDFLAGS) start.o makedir.o -o makedir.coff
	../bin/coff2noff makedir.coff makedir

vmtorture.o: vmtorture.c
	$(CC) $(CFLAGS) -c vmtorture.c
vmtorture: vmtorture.o start.o
	$(LD) $(LDFLAGS) start.o vmtorture.o -o vmtorture.coff
	../bin/coff2noff vmtorture.coff vmtorture

qmatmult.o: qmatmult.c
	$(CC) $(CFLAGS) -c qmatmult.c
qmatmult: qmatmult.o start.o
	$(LD) $(LDFLAGS) start.o qmatmult.o -o qmatmult.coff
	../bin/coff2noff qmatmult.coff qmatmult

qsort.o: qsort.c
	$(CC) $(CFLAGS) -c qsort.c
qsort: qsort.o start.o
	$(LD) $(LDFLAGS) start.o qsort.o -o qsort.coff
	../bin/coff2noff qsort.coff qsort

Xkid.o: Xkid.c
	$(CC) $(CFLAGS) -c Xkid.c
Xkid: Xkid.o start.o
	$(LD) $(LDFLAGS) start.o Xkid.o -o Xkid.coff
	../bin/coff2noff Xkid.coff Xkid
