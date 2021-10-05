all: swapon swapoff

swapon:	swapon.o
	$(CC) -o swapon swapon.o

swapon.o:	swapon.c
	$(CC) -o swapon.o -c swapon.c

swapoff:	swapoff.o
	$(CC) -o swapoff swapoff.o

swapoff.o:	swapoff.c
	$(CC) -o swapoff.o -c swapoff.c

clean:
	rm -f swapon swapoff swapon.o swapoff.o

