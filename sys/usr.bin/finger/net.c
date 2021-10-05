/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Tony Nardo of the Johns Hopkins University/Applied Physics Lab.
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
/*static char sccsid[] = "from: @(#)net.c	5.5 (Berkeley) 6/1/90";*/
char net_rcsid[] = "$Id: net.c,v 1.9 1999/09/14 10:51:11 dholland Exp $";
#endif /* not lint */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include "finger.h"

#if ! defined(FINGER_TIMEOUT) || FINGER_TIMEOUT < 1
# define FINGER_TIMEOUT 5
#endif

static void trivial_alarm(int sig) {
	/* Just to trigger EINTR, and to later use it. */
	return;
}

int netfinger(const char *name) {
	register FILE *fp;
	register int c, sawret, ateol;
	struct addrinfo hints, *result, *resptr;
	struct servent *sp;
	struct sockaddr_storage sn;
	struct sigaction sigact, oldsigact;
	int s, status;
	char *host;

	host = strrchr(name, '@');
	if (!host) return 1;
	*host++ = '\0';

	memset(&sn, 0, sizeof(sn));

	sp = getservbyname("finger", "tcp");
	if (!sp) {
		eprintf("finger: tcp/finger: unknown service\n");
		return 1;
	}
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags  = AI_CANONNAME | AI_ADDRCONFIG;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	sigact.sa_handler = trivial_alarm;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;

	status = getaddrinfo(host, "finger", &hints, &result);
	if (status != 0) {
		eprintf("finger: unknown host: %s\n", host);
		eprintf("getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	}

	for ( resptr = result; resptr; resptr = resptr->ai_next) {

		if ((s = socket(resptr->ai_family, resptr->ai_socktype,
				resptr->ai_protocol)) < 0)
			continue;


		/* print hostname before connecting, in case it takes a while */
		/* This should probably be removed. */
		/* xprintf("[%s]\n", result->ai_canonname); */

		sigaction(SIGALRM, &sigact, &oldsigact);
		alarm(FINGER_TIMEOUT);

		if (connect(s, resptr->ai_addr, resptr->ai_addrlen) < 0) {
			if ( errno == EINTR )
				errno = ETIMEDOUT;
			close(s);
			continue;
		}

		alarm(0);
		sigaction(SIGALRM, &oldsigact, NULL);

		/* Connection is now established.
		 * Assemble the gained information. */
		memcpy(&sn, resptr->ai_addr, resptr->ai_addrlen);
		break;
	}

	freeaddrinfo(result);

	if ( resptr == NULL ) {
		/* Last error is still providing the correct clue. */
		eprintf("finger: connect: %s\n", strerror(errno));
		return 1;
	}

	/* -l flag for remote fingerd  */
	if (lflag) write(s, "/W ", 3);

	/* send the name followed by <CR><LF> */
	write(s, name, strlen(name));
	write(s, "\r\n", 2);

	/*
	 * Read from the remote system; once we're connected, we assume some
	 * data.  If none arrives, we hang until the user interrupts.
	 *
	 * If we see a <CR> or a <CR> with the high bit set, treat it as
	 * a newline; if followed by a newline character, only output one
	 * newline.
	 *
	 * Text is sent to xputc() for printability analysis.
	 */ 
	fp = fdopen(s, "r");
	if (!fp) {
		eprintf("finger: fdopen: %s\n", strerror(errno));
		close(s);
		return 1;
	}

	sawret = 0;
	ateol = 1;
	while ((c = getc(fp)) != EOF) {
		c &= 0xff;
		if (c == ('\r'|0x80) || c == ('\n'|0x80)) c &= 0x7f;
		if (c == '\r') {
			sawret = ateol = 1;
			xputc('\n');
		} 
		else if (sawret && c == '\n') {
			sawret = 0;
			/* don't print */
		}
		else {
			if (c == '\n') ateol = 1;
			sawret = 0;
			xputc(c);
		}
	}
	if (!ateol) xputc('\n');
	fclose(fp);

	return 0;
}
