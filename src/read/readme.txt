This is a System V-flavored set of userland tools, designed for the Lunaris
Project.  They are conformed, as far as reasonably possible, to the versions
described in the manual pages of a specific implementation of Unix System V
Release 2, as published by AT&T in 1986.  Some functionality from other 
implementations, especially BSD, has been retained.

Some of the code was taken from periodical raids of the OpenBSD and NetBSD
source trees from circa 1999 to present.  In some cases I have augmented or 
replaced pieces of said code.

Most of the system builds with clang and musl, which are the intended system
compiler and libc, but some stuff is currently reliant on things which are not
provided by that combination yet.  We're working on it.

Files in the "internal" folder are used for maintenance of the system and are
not intended for general use.
