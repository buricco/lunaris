/*
 * Copyright (c) 1985, 1989 Regents of the University of California.
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

void settick(void);
void disconnect(void);

void setascii(void);
void setbell(void);
void setbinary(void);
void setdebug(int argc, char *argv[]);
void setform(void);
void setglob(void);
void sethash(int argc, char *argv[]);
void setipany(void);
void setipv4(void);
void setipv6(void);
void setmode(void);
void setport(void);
void setprompt(void);
void setstruct(void);
void settenex(void);
void settrace(void);
void settype(int argc, char *argv[]);
void setverbose(void);
void restart(int argc, char *argv[]);
void reget(int argc, char *argv[]);
void syst(void);
void cd(int argc, char *argv[]);
void lcd(int argc, char *argv[]);
void delete_cmd(int argc, char *argv[]);
void mdelete(int argc, char *argv[]);
void user(int argc, char *argv[]);
void ls(int argc, char *argv[]);
void mls(int argc, char *argv[]);
void get(int argc, char *argv[]);
void mget(int argc, char *argv[]);
void help(int argc, char *argv[]);
void append(int argc, char *argv[]);
void put(int argc, char *argv[]);
void mput(int argc, char *argv[]);
void renamefile(int argc, char *argv[]);
void status(void);
void quote(int argc, char *argv[]);
void rmthelp(int argc, char *argv[]);
void shell(const char *arg);
void site(int argc, char *argv[]);
void pwd(void);
void makedir(int argc, char *argv[]);
void removedir(int argc, char *argv[]);
void setcr(void);
void setqc(void);
void account(int argc, char *argv[]);
void doproxy(int argc, char *argv[]);
void reset(void);
void setcase(void);
void setntrans(int argc, char *argv[]);
void setnmap(int argc, char *argv[]);
void setsunique(void);
void setrunique(void);
void cdup(void);
void macdef(int argc, char *argv[]);
void sizecmd(int argc, char *argv[]);
void modtime(int argc, char *argv[]);
void newer(int argc, char *argv[]);
void rmtstatus(int argc, char *argv[]);
void do_chmod(int argc, char *argv[]);
void do_umask(int argc, char *argv[]);
void idle_cmd(int argc, char *argv[]);
void setpassive(void);
