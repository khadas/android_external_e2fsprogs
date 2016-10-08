/* vi: set sw=4 ts=4: */
/*
 * Utility routines.
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2006 Rob Landley
 * Copyright (C) 2006 Denys Vlasenko
 *
 * Licensed under GPLv2, see file LICENSE in this source tree.
 */

/* We need to have separate xfuncs.c and xfuncs_printf.c because
 * with current linkers, even with section garbage collection,
 * if *.o module references any of XXXprintf functions, you pull in
 * entire printf machinery. Even if you do not use the function
 * which uses XXXprintf.
 *
 * xfuncs.c contains functions (not necessarily xfuncs)
 * which do not pull in printf, directly or indirectly.
 * xfunc_printf.c contains those which do.
 */

//#include "libbb.h"
#include "xfuncs_printf.h"

ssize_t safe_read(int fd, void *buf, size_t count)
{
        ssize_t n;

        do {
                n = read(fd, buf, count);
        } while (n < 0 && errno == EINTR);

        return n;
}

/*
 * Read all of the supplied buffer from a file.
 * This does multiple reads as necessary.
 * Returns the amount read, or -1 on an error.
 * A short read is returned on an end of file.
 */
ssize_t full_read(int fd, void *buf, size_t len)
{
        ssize_t cc;
        ssize_t total;

        total = 0;

        while (len) {
                cc = safe_read(fd, buf, len);

                if (cc < 0) {
                        if (total) {
                                /* we already have some! */
                                /* user can do another read to know the error code */
                                return total;
                        }
                        return cc; /* read() returns -1 on failure. */
                }
                if (cc == 0)
                        break;
                buf = ((char *)buf) + cc;
                total += cc;
                len -= cc;
        }

        return total;
}

/* All the functions starting with "x" call bb_error_msg_and_die() if they
 * fail, so callers never need to check for errors.  If it returned, it
 * succeeded. */

/* dmalloc provides variants of these that do abort() on failure.
 * Since dmalloc's prototypes overwrite the impls here as they are
 * included after these prototypes in libbb.h, all is well.
 */
// Warn if we can't allocate size bytes of memory.
void* /**/ malloc_or_warn(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL && size != 0)
		;//bb_error_msg("%s", bb_msg_memory_exhausted);
	return ptr;
}

// Die if we can't allocate size bytes of memory.
void* xmalloc(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL && size != 0)
		;//bb_error_msg_and_die("%s", bb_msg_memory_exhausted);
	return ptr;
}

// Die if we can't resize previously allocated memory.  (This returns a pointer
// to the new memory, which may or may not be the same as the old memory.
// It'll copy the contents to a new chunk and free the old one if necessary.)
void* /**/ xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if (ptr == NULL && size != 0)
		;//bb_error_msg_and_die("%s", bb_msg_memory_exhausted);
	return ptr;
}

// Die if we can't allocate and zero size bytes of memory.
void* /**/ xzalloc(size_t size)
{
	void *ptr = xmalloc(size);
	memset(ptr, 0, size);
	return ptr;
}

// Die if we can't copy a string to freshly allocated memory.
char* /**/ xstrdup(const char *s)
{
	char *t;

	if (s == NULL)
		return NULL;

	t = strdup(s);

	if (t == NULL)
		;//bb_error_msg_and_die("%s", bb_msg_memory_exhausted);

	return t;
}

// Die if we can't allocate n+1 bytes (space for the null terminator) and copy
// the (possibly truncated to length n) string into it.
char* /**/ xstrndup(const char *s, int n)
{
	int m;
	char *t;

	if (1 && s == NULL)
		;//bb_error_msg_and_die("xstrndup bug");

	/* We can just xmalloc(n+1) and strncpy into it, */
	/* but think about xstrndup("abc", 10000) wastage! */
	m = n;
	t = (char*) s;
	while (m) {
		if (!*t) break;
		m--;
		t++;
	}
	n -= m;
	t = xmalloc(n + 1);
	t[n] = '\0';

	return memcpy(t, s, n);
}

// Die if we can't open a file and return a FILE* to it.
// Notice we haven't got xfread(), This is for use with fscanf() and friends.
FILE* /**/ xfopen(const char *path, const char *mode)
{
	FILE *fp = fopen(path, mode);
	if (fp == NULL)
		;//bb_perror_msg_and_die("can't open '%s'", path);
	return fp;
}

// Die if we can't open a file and return a fd.
int /**/ xopen3(const char *pathname, int flags, int mode)
{
	int ret;

	ret = open(pathname, flags, mode);
	if (ret < 0) {
		//bb_perror_msg_and_die("can't open '%s'", pathname);
	}
	return ret;
}

// Die if we can't open a file and return a fd.
int /**/ xopen(const char *pathname, int flags)
{
	return xopen3(pathname, flags, 0666);
}

// Warn if we can't open a file and return a fd.
int /**/ open3_or_warn(const char *pathname, int flags, int mode)
{
	int ret;

	ret = open(pathname, flags, mode);
	if (ret < 0) {
		//bb_perror_msg("can't open '%s'", pathname);
	}
	return ret;
}

// Warn if we can't open a file and return a fd.
int /**/ open_or_warn(const char *pathname, int flags)
{
	return open3_or_warn(pathname, flags, 0666);
}

/* Die if we can't open an existing file readonly with O_NONBLOCK
 * and return the fd.
 * Note that for ioctl O_RDONLY is sufficient.
 */
int  xopen_nonblocking(const char *pathname)
{
	return xopen(pathname, O_RDONLY | O_NONBLOCK);
}

int  xopen_as_uid_gid(const char *pathname, int flags, uid_t u, gid_t g)
{
	int fd;
	uid_t old_euid = geteuid();
	gid_t old_egid = getegid();

	xsetegid(g);
	xseteuid(u);

	fd = xopen(pathname, flags);

	xseteuid(old_euid);
	xsetegid(old_egid);

	return fd;
}

void  xunlink(const char *pathname)
{
	if (unlink(pathname))
		;//bb_perror_msg_and_die("can't remove file '%s'", pathname);
}

void  xrename(const char *oldpath, const char *newpath)
{
	if (rename(oldpath, newpath))
		;//bb_perror_msg_and_die("can't move '%s' to '%s'", oldpath, newpath);
}

int  rename_or_warn(const char *oldpath, const char *newpath)
{
	int n = rename(oldpath, newpath);
	if (n)
		;//bb_perror_msg("can't move '%s' to '%s'", oldpath, newpath);
	return n;
}

void  xpipe(int filedes[2])
{
	if (pipe(filedes))
		;//bb_perror_msg_and_die("can't create pipe");
}

void  xdup2(int from, int to)
{
	if (dup2(from, to) != to)
		;//bb_perror_msg_and_die("can't duplicate file descriptor");
}

void  xsetegid(gid_t egid)
{
	if (setegid(egid)) ;//bb_perror_msg_and_die("setegid");
}

// Die with an error message if we can't set uid.  (See xsetgid() for why.)
void  xsetuid(uid_t uid)
{
	if (setuid(uid)) ;//bb_perror_msg_and_die("setuid");
}

void  xseteuid(uid_t euid)
{
	if (seteuid(euid)) ;//bb_perror_msg_and_die("seteuid");
}
