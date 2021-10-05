file:	apprentice.o apptype.o ascmagic.o asctime_r.o asprintf.o buffer.o \
	cdf.o cdf_time.o compress.o ctime_r.o der.o dprintf.o encoding.o \
	file.o fmtcheck.o fsmagic.o funcs.o getline.o getopt_long.o \
	gmtime_r.o is_csv.o is_json.o is_tar.o localtime_r.o magic.o pread.o \
	print.o readcdf.o readelf.o seccomp.o softmagic.o strcasestr.o \
	strlcat.o strlcpy.o vasprintf.o
	cc -o file apprentice.o apptype.o ascmagic.o asctime_r.o asprintf.o \
		buffer.o cdf.o cdf_time.o compress.o ctime_r.o der.o \
		dprintf.o encoding.o file.o fmtcheck.o fsmagic.o funcs.o \
		getline.o getopt_long.o gmtime_r.o is_csv.o is_json.o \
		is_tar.o localtime_r.o magic.o pread.o print.o readcdf.o \
		readelf.o seccomp.o softmagic.o strcasestr.o strlcat.o \
		strlcpy.o vasprintf.o -lz -lbz2

apprentice.o:	apprentice.c file.h magic.h
	cc -c -o apprentice.o apprentice.c

apptype.o:	apptype.c file.h
	cc -c -o apptype.o apptype.c

ascmagic.o:	ascmagic.c file.h magic.h
	cc -c -o ascmagic.o ascmagic.c

asctime_r.o:	asctime_r.c file.h
	cc -c -o asctime_r.o asctime_r.c

asprintf.o:	asprintf.c file.h
	cc -c -o asprintf.o asprintf.c

buffer.o:	buffer.c file.h magic.h
	cc -c -o buffer.o buffer.c

cdf.o:	cdf.c cdf.h file.h
	cc -c -o cdf.o cdf.c

cdf_time.o:	cdf_time.c cdf.h file.h
	cc -c -o cdf_time.o cdf_time.c

compress.o:	compress.c file.h magic.h
	cc -c -o compress.o compress.c

ctime_r.o:	ctime_r.c file.h
	cc -c -o ctime_r.o ctime_r.c

der.o:	der.c der.h file.h magic.h
	cc -c -o der.o der.c

dprintf.o:	dprintf.c file.h
	cc -c -o dprintf.o dprintf.c

encoding.o:	encoding.c file.h magic.h
	cc -c -o encoding.o encoding.c

file.o:	file.c config.h file.h file_opts.h magic.h mygetopt.h
	cc -c -o file.o file.c

fmtcheck.o:	fmtcheck.c file.h
	cc -c -o fmtcheck.o fmtcheck.c

fsmagic.o:	fsmagic.c file.h magic.h
	cc -c -o fsmagic.o fsmagic.c

funcs.o:	funcs.c file.h magic.h
	cc -c -o funcs.o funcs.c

getline.o:	getline.c file.h
	cc -c -o getline.o getline.c

getopt_long.o:	getopt_long.c file.h mygetopt.h
	cc -c -o getopt_long.o getopt_long.c

gmtime_r.o:	gmtime_r.c file.h
	cc -c -o gmtime_r.o gmtime_r.c

is_csv.o:	is_csv.c file.h magic.h
	cc -c -o is_csv.o is_csv.c

is_json.o:	is_json.c file.h magic.h
	cc -c -o is_json.o is_json.c

is_tar.o:	is_tar.c file.h magic.h tar.h
	cc -c -o is_tar.o is_tar.c

localtime_r.o:	localtime_r.c file.h
	cc -c -o localtime_r.o localtime_r.c

magic.o:	magic.c file.h magic.h
	cc -c -o magic.o magic.c

pread.o:	pread.c file.h
	cc -c -o pread.o pread.c

print.o:	print.c cdf.h file.h
	cc -c -o print.o print.c

readcdf.o:	readcdf.c cdf.h file.h magic.h
	cc -c -o readcdf.o readcdf.c

readelf.o:	readelf.c elfclass.h file.h magic.h readelf.h
	cc -c -o readelf.o readelf.c

seccomp.o:	seccomp.c file.h
	cc -c -o seccomp.o seccomp.c

softmagic.o:	softmagic.c der.h file.h magic.h
	cc -c -o softmagic.o softmagic.c

strcasestr.o:	strcasestr.c file.h
	cc -c -o strcasestr.o strcasestr.c

strlcat.o:	strlcat.c file.h
	cc -c -o strlcat.o strlcat.c

strlcpy.o:	strlcpy.c file.h
	cc -c -o strlcpy.o strlcpy.c

vasprintf.o:	vasprintf.c file.h
	cc -c -o vasprintf.o vasprintf.c

clean:
	rm -f file apprentice.o apptype.o ascmagic.o asctime_r.o asprintf.o \
		buffer.o cdf.o cdf_time.o compress.o ctime_r.o der.o \
		dprintf.o encoding.o file.o fmtcheck.o fsmagic.o funcs.o \
		getline.o getopt_long.o gmtime_r.o is_csv.o is_json.o \
		is_tar.o localtime_r.o magic.o pread.o print.o readcdf.o \
		readelf.o seccomp.o softmagic.o strcasestr.o strlcat.o \
		strlcpy.o vasprintf.o
