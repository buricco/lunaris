robots:	auto.o extern.o flush_in.o init_field.o main.o make_level.o move.o move_robs.o play_level.o query.o rnd_pos.o score.o
	cc -o robots auto.o extern.o flush_in.o init_field.o main.o make_level.o move.o move_robs.o play_level.o query.o rnd_pos.o score.o -lcurses

auto.o:	auto.c robots.h
	cc -c -o auto.o auto.c

extern.o:	extern.c robots.h
	cc -c -o extern.o extern.c

flush_in.o:	flush_in.c robots.h
	cc -c -o flush_in.o flush_in.c

init_field.o:	init_field.c robots.h
	cc -c -o init_field.o init_field.c

main.o:	main.c robots.h
	cc -c -o main.o main.c

make_level.o:	make_level.c robots.h
	cc -c -o make_level.o make_level.c

move.o:	move.c robots.h
	cc -c -o move.o move.c

move_robs.o:	move_robs.c robots.h
	cc -c -o move_robs.o move_robs.c

play_level.o:	play_level.c robots.h
	cc -c -o play_level.o play_level.c

query.o:	query.c robots.h
	cc -c -o query.o query.c

rnd_pos.o:	rnd_pos.c robots.h
	cc -c -o rnd_pos.o rnd_pos.c

score.o:	score.c robots.h
	cc -c -o score.o score.c

clean:
	rm -f robots auto.o extern.o flush_in.o init_field.o main.o make_level.o move.o move_robs.o play_level.o query.o rnd_pos.o score.o
