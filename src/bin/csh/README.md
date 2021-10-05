# csh-linux
openbsd's csh patched for linux 2015-12-28

This includes several debian patches, mainly for glob.c and proc.c,
and parts from libbsd/openbsd. I included them so it shouldn't need
any extra libraries.

To make use gnu make:

    make -f Makefile.gnu
    strip -s -R .note -R .comment csh
    (sudo) install -o root -g root -m 555 csh /bin/csh
    (sudo) echo /bin/csh >> /etc/shells
    (sudo) install -o root -g root -m 444 csh.1 /usr/share/man/man1/csh.1
    (sudo) gzip -9v /usr/share/man/man1/csh.1


Optional make a static version to keep somehwere safe:

    gcc -static -flto -o csh.static *.o
    but, on linux with glibc static linking is broken and won't be fixed.
    It still works though but user lookup (things like ~ expansion) are
    probably broken.

