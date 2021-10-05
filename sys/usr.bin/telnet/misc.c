/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
/* from: static char sccsid[] = "@(#)misc.c    8.1 (Berkeley) 6/4/93"; */
/* from: static char rcsid[] = "$NetBSD: misc.c,v 1.5 1996/02/24 01:15:25 jtk Exp $"; */
static char rcsid[] = "$OpenBSD: misc.c,v 1.1 2005/05/24 03:43:56 deraadt Exp $";
#endif /* not lint */

/* $KTH: misc.c,v 1.15 2000/01/25 23:24:58 assar Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include "misc.h"
#include "auth.h"
#include "encrypt.h"


const char *RemoteHostName;
const char *LocalHostName;
char *UserNameRequested = 0;
int ConnectedCount = 0;

void
auth_encrypt_init(const char *local, const char *remote, const char *name,
		  int server)
{
    RemoteHostName = remote;
    LocalHostName = local;
#ifdef AUTHENTICATION
    auth_init(name, server);
#endif
#ifdef ENCRYPTION
    encrypt_init(name, server);
#endif
    if (UserNameRequested) {
	free(UserNameRequested);
	UserNameRequested = 0;
    }
}

void
auth_encrypt_user(const char *name)
{
    if (UserNameRequested)
	free(UserNameRequested);
    UserNameRequested = name ? strdup(name) : 0;
}

void
auth_encrypt_connect(int cnt)
{
}

void
printd(const unsigned char *data, int cnt)
{
    if (cnt > 16)
	cnt = 16;
    while (cnt-- > 0) {
	printf(" %02x", *data);
	++data;
    }
}

/* This is stolen from libroken; it's the only thing actually needed from
 * libroken.
 */
void
esetenv(const char *var, const char *val, int rewrite)
{
    if (setenv ((char *)var, (char *)val, rewrite))
        errx (1, "failed setting environment variable %s", var);
}
