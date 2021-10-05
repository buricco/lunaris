logger:	logger.o
	$(CC) -o logger logger.o

logger.o:	logger.c
	$(CC) -c -o logger.o logger.c

clean:
	rm -f logger logger.o

install:
	install -d -m 755 logger /usr/bin

