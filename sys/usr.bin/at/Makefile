all:	at atrun

at:	at.o
	cc -o at at.o

at.o:	at.c
	cc -c -o at.o at.c

atrun:	atrun.o
	cc -o atrun atrun.o

atrun.o:	atrun.c
	cc -c -o atrun.o atrun.c

clean:
	rm -f at atrun at.o atrun.o
