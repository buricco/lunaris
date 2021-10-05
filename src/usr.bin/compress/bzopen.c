/*
 * Copyright (c) 2020 S. V. Nickolas.
 * Adapted from nullopen.c
 * Copyright (c) 2003 Can Erkin Acar
 * Copyright (c) 1997 Michael Shalayeff
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <bzlib.h>

#include "compress.h"
#include "bzopen.h"

typedef struct
{
 unsigned in, out;
 int fd;
 FILE *file;
 BZFILE *bzfile;
 char mode;
 int gotmagic;
} bzcookie;

void *bzip_ropen(int fd, char *name, int gotmagic)
{
 bzcookie *cookie;
 int e;
 
 if (fd<0) return 0;
 
 if (!(cookie=calloc(1,sizeof(bzcookie)))) return 0;
 
 cookie->fd=fd;
 cookie->file=fdopen(fd,"rb");
 if (!(cookie->file))
 {
  free(cookie);
  return 0;
 }
 
 cookie->bzfile=BZ2_bzReadOpen(&e, cookie->file, 0, 0, 0, 0);
 if (e!=BZ_OK)
 {
  free(cookie);
  return 0;
 }
 
 cookie->mode='r';
 cookie->gotmagic=gotmagic;
 cookie->in=cookie->out=0; 
 return cookie;
}

void *bzip_wopen(int fd, char *name, int bits, u_int32_t mtime)
{
 bzcookie *cookie;
 int e;
 
 if (fd<0) return 0;

 if (!(cookie=calloc(1,sizeof(bzcookie)))) return 0;
 
 cookie->fd=fd;
 cookie->file=fdopen(fd,"wb");
 if (!(cookie->file))
 {
  free(cookie);
  return 0;
 }
 
 cookie->bzfile=BZ2_bzWriteOpen(&e, cookie->file, bits, 0, 30);
 if (e!=BZ_OK)
 {
  free(cookie);
  return 0;
 }
 
 cookie->mode='w';
 cookie->gotmagic=0;
 cookie->in=cookie->out=0;
 return cookie;
}

int bzip_close(void *cookie, struct z_info *info, const char *name, struct stat *sb)
{
 bzcookie *c=(bzcookie*) cookie;
 int e;
 
 if (!c) return -1;
 
 if (c->mode=='r')
 {
  BZ2_bzReadClose(&e, c->bzfile);
  c->in=c->out=0;
 }
 else if (c->mode=='w')
 {
  BZ2_bzWriteClose(&e, c->bzfile, 0, &(c->in), &(c->out));
 }
 else return -1;
 
 if (info)
 {
  info->mtime=0;
  info->crc=(u_int32_t) -1;
  info->hlen=0;
  info->total_in=c->in;
  info->total_in=c->out;
 }
 
 setfile(name,c->fd,sb);
 fclose(c->file);
 close(c->fd);
 free(c);
 
 return 0;
}

int bzip_flush(void *cookie, int flush)
{
 bzcookie *c=(bzcookie*) cookie;
 
 if ((!c)||((c->mode)!='w'))
 {
  errno=EBADF;
  return -1;
 }

 return 0;
}

int bzip_read(void *cookie, char *buf, int len)
{
 bzcookie *c=(bzcookie*) cookie;
 int e, f;
 
 f=BZ2_bzRead(&e, c->bzfile, buf, len);
 if (e!=BZ_OK) return -1;
 
 c->in+=len;
 c->out+=len;
 
 return len;
}

int bzip_write(void *cookie, const char *buf, int len)
{
 bzcookie *c=(bzcookie*) cookie;
 int e;
 
 BZ2_bzWrite(&e, c->bzfile, (char*) buf, len);
 if (e!=BZ_OK) return -1;
 
 return len;
}


