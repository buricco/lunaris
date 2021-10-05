/*-
 * Copyright (c) 1991, 1993, 1994
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
 *
 *	@(#)extern.h	8.3 (Berkeley) 4/16/94
 */

#include <sys/cdefs.h>

void	 brace_subst (char *, char **, char *, int);
void	*emalloc (unsigned int);
PLAN	*find_create (char ***);
int	 find_execute (PLAN *, char **);
PLAN	*find_formplan (char **);
PLAN	*not_squish (PLAN *);
PLAN	*or_squish (PLAN *);
PLAN	*paren_squish (PLAN *);
struct stat;
void	 printlong (char *, char *, struct stat *);
int	 queryuser (char **);

PLAN	*c_atime (char *);
PLAN	*c_ctime (char *);
PLAN	*c_depth (void);
PLAN	*c_exec (char ***, int);
PLAN	*c_follow (void);
PLAN	*c_fstype (char *);
PLAN	*c_group (char *);
PLAN	*c_inum (char *);
PLAN	*c_links (char *);
PLAN	*c_ls (void);
PLAN	*c_name (char *);
PLAN	*c_newer (char *);
PLAN	*c_nogroup (void);
PLAN	*c_nouser (void);
PLAN	*c_path (char *);
PLAN	*c_perm (char *);
PLAN	*c_print (void);
PLAN	*c_prune (void);
PLAN	*c_size (char *);
PLAN	*c_type (char *);
PLAN	*c_user (char *);
PLAN	*c_xdev (void);
PLAN	*c_openparen (void);
PLAN	*c_closeparen (void);
PLAN	*c_mtime (char *);
PLAN	*c_not (void);
PLAN	*c_or (void);

extern int ftsoptions, isdeprecated, isdepth, isoutput, isxargs;
