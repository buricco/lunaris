pivot_root:	pivot_root.o
	$(CC) -o pivot_root pivot_root.o

pivot_root.o:	pivot_root.c
	$(CC) -c -o pivot_root.o pivot_root.c

clean:
	rm -f pivot_root pivot_root.o

install:
	install -d -m 755 pivot_root /bin

