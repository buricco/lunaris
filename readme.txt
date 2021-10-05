It is possible that since the last time I touched this code in March 2021, it
bitrotted.  See some of the documentation for details.  I have removed the
unmodified trees (see read/imported.txt for more information).

I am trying to create an operating system using the Linux kernel, clang and
musl, but I got wedged in the process of building the freestanding temporary
system (something similar to LFS stage 1).  This is where the code stood when
I last touched it, minus internal tools and the above-mentioned unmodified
trees.

Since I haven't gotten even this much of the system ready, I have not even
tried to go beyond the "core" level, i.e., to add X Window, Motif and the CDE,
which are also major parts of my plan to create a system similar to "System V
Unix Release 4.2" running on Linux.  Some tools are unfinished (the sysvinit 
emulation layer on top of Arachsys init, especially).

S. V. Nickolas
Oct. 5, 2021
