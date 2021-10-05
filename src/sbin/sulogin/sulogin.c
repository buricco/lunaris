/*
 * sulogin - the single user mode login program.
 *
 * This is a program to force the console user to login under a root
 * account before a shell is started.
 *
 * The supported command line options match those used with Miquel van
 * Smoorenburg's implementation of sulogin.
 *
 * Copyright (c) 2003,2012 by Solar Designer <solar at owl.openwall.com>
 * Adaptations for Lunaris
 * Copyright (c) 2020 by S. V. Nickolas <usotsuki at buric.co>
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted.
 * 
 * There is ABSOLUTELY NO WARRANTY, express or implied.
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <termios.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <pwd.h>
#include <shadow.h>
#include <crypt.h>

extern char *__progname;

#define LINE_BUFFER_SIZE		0x100

static int opt_profile = 0;
static unsigned int opt_timeout = 0;

static sigjmp_buf jmp_alarm;

typedef enum {
	S_WORKING, S_PASSED, S_FAILED = 64, S_TIMEOUT = 65, S_EOF = 66
} sulogin_state_t;

static void handle_alarm(int signum)
{
	(void) signum;
	siglongjmp(jmp_alarm, 1);
}

static int getline_fd(char *line, int size, int fd)
{
	char *p, *e, c;
	int retval;

	if (sigsetjmp(jmp_alarm, 1)) {
		alarm(0);
		signal(SIGALRM, SIG_DFL);
		return -S_TIMEOUT;
	}

	signal(SIGALRM, handle_alarm);
	alarm(opt_timeout);

	p = line;
	e = line + (size - 1);
	while ((retval = read(fd, &c, 1)) == 1) {
		if (c == '\n') break;
		if (p < e) *p++ = c;
	}
	*p = '\0';

	alarm(0);
	signal(SIGALRM, SIG_DFL);

	if (retval <= 0) {
		if (retval == 0) return -S_EOF;
		return -S_FAILED;
	}

	return p - line;
}

static int putline_fd(const char *line, int fd)
{
	const char *p;
	int count, block;

	p = line;
	count = strlen(line);
	while (count > 0) {
		block = write(fd, p, count);

		if (block < 0) return block;
		if (!block) return p - line;

		p += block;
		count -= block;
	}

	return p - line;
}

static struct passwd *getspwnam(const char *user)
{
	struct passwd *pw;
	struct spwd *spw;

	if ((pw = getpwnam(user)) &&
	    !strcmp(pw->pw_passwd, "x") &&
	    (spw = getspnam(user)))
		pw->pw_passwd = spw->sp_pwdp;
	endpwent();

	return pw;
}

/*
 * As currently implemented, we don't avoid timing leaks for valid vs. not
 * usernames and hashes.  However, we do reduce timing leaks for root vs.
 * non-root accounts.
 */
static int valid(struct passwd *pw, const char *pass)
{
	int retval;

	if (!pw)
		return 0;

	retval = !pass[0] && !pw->pw_passwd[0];

	if (!retval && strlen(pw->pw_passwd) >= 13) {
		const char *hash = crypt(pass, pw->pw_passwd);
		if (hash)
			retval = !strcmp(hash, pw->pw_passwd);
	}

	if (pw->pw_uid != 0)
		retval = 0;

	memset(pw->pw_passwd, 0, strlen(pw->pw_passwd));

	return retval;
}

static void sushell(struct passwd *pw)
{
	char *home, *shell, *argv0;

	if (chdir(home = pw->pw_dir))
		chdir(home = "/");

	if ((shell = pw->pw_shell) && (argv0 = strrchr(shell, '/'))) {
		if (opt_profile) {
			if (!(argv0 = strdup(argv0)))
				return;
			argv0[0] = '-';
		} else
			argv0++;
	} else {
		shell = "/bin/sh";
		argv0 = opt_profile ? "-sh" : "sh";
	}

	setenv("HOME", home, 1);
	setenv("LOGNAME", pw->pw_name, 1);
	setenv("USER", pw->pw_name, 1);

	setenv("SHELL", shell, 1);
	if (!opt_profile)
		setenv("SHLVL", "0", 1);

	execl(shell, argv0, NULL);
	perror(shell);
}

static int sulogin(void)
{
	sulogin_state_t state;
	int retval;
	int tty, in, out;
	int tty_saved, tty_changed;
	struct termios s, t;
	char pass[LINE_BUFFER_SIZE];
	struct passwd *pw;

	state = S_WORKING;

	tty = in = out = open("/dev/tty", O_RDWR);
	if (tty < 0) {
		in = STDIN_FILENO;
		out = STDERR_FILENO;
	}

	tty_saved = (tcgetattr(in, &s) == 0);
	tty_changed = 0;

	pw = NULL; /* never used */

	while (1) {
		putline_fd("\nType Ctrl-D to proceed with normal startup\n"
                     "(or give root password for system maintenance): ", out);
		if (tty_saved) {
			t = s;
			t.c_lflag &= ~(ECHO|ISIG);
			tty_changed = (tcsetattr(in, TCSAFLUSH, &t) == 0);
		}
		if ((retval = getline_fd(pass, sizeof(pass), in)) < 0)
			state = -retval;

		if (tty_changed) {
			if (state == S_WORKING)
				putline_fd("\n", out);
			tcsetattr(in, TCSAFLUSH, &s);
			tty_changed = 0;
		}

		if (state != S_WORKING)
			break;

		pw = getspwnam("root");

		if (valid(pw, pass)) {
			state = S_PASSED;
			break;
		}

		putline_fd("Login incorrect.\n", out);
	}

	if (state == S_TIMEOUT)
		putline_fd("\nTimed out.\n", out);

	if (state == S_FAILED || state == S_EOF)
		putline_fd("\n", out);

	putline_fd("\n", out);
    
    if (state == S_PASSED) putline_fd("Entering system maintenance mode\n\n", out);

	if (tty >= 0)
		close(tty);

	if (state == S_PASSED) {
		sushell(pw);
		return S_FAILED;
	}

	return state;
}

static void usage(void)
{
	putline_fd("Usage: ", STDERR_FILENO);
	putline_fd(__progname, STDERR_FILENO);
	putline_fd(" [-p] [-t timeout]\n", STDERR_FILENO);
}

static void parse(int argc, char **argv)
{
	int do_usage;
	int c;
	long value;
	char *error;

	do_usage = 0;
	while ((c = getopt(argc, argv, "pt:")) != -1) {
		switch (c) {
		case 'p':
			opt_profile = 1;
			break;

		case 't':
			errno = 0;
			value = strtol(optarg, &error, 10);
			if (errno || !*optarg || *error ||
			    value < 0 || value > INT_MAX)
				do_usage = 1;
			else
				opt_timeout = value;
			break;

		default:
			do_usage = 1;
		}
	}

	if (do_usage || optind != argc)
		usage();
}

int main(int argc, char **argv)
{
	parse(argc, argv);

	if (geteuid() != 0) {
		putline_fd("Error: must be started as superuser\n", STDERR_FILENO);
		return S_FAILED;
	}

	return sulogin();
}
