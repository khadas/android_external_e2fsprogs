#ifndef __XFUNCS_PRINTF__
#define __XFUNCS_PRINTF__
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

ssize_t safe_read(int fd, void *buf, size_t count);
ssize_t full_read(int fd, void *buf, size_t len);

void*  malloc_or_warn(size_t size);
void*  xmalloc(size_t size);
void*  xrealloc(void *ptr, size_t size);
void*  xzalloc(size_t size);
char*  xstrdup(const char *s);
char* /**/ xstrndup(const char *s, int n);
FILE* /**/ xfopen(const char *path, const char *mode);
int /**/ xopen3(const char *pathname, int flags, int mode);
int /**/ xopen(const char *pathname, int flags);
int  xopen_nonblocking(const char *pathname);
int  xopen_as_uid_gid(const char *pathname, int flags, uid_t u, gid_t g);
void  xunlink(const char *pathname);
void  xrename(const char *oldpath, const char *newpath);

int  rename_or_warn(const char *oldpath, const char *newpath);
void  xpipe(int filedes[2]);
void  xdup2(int from, int to);
void  xsetegid(gid_t egid);
void  xsetuid(uid_t uid);
void  xseteuid(uid_t euid);

#endif
