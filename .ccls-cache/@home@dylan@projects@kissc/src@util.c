#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>   /* malloc, size_t */
#include <stdio.h>    /* printf */
#include <limits.h>   /* PATH_MAX  */
#include <string.h>   /* strncpy */
#include <sys/stat.h> /* mkdir */
#include <errno.h>    /* errno, EEXIST, S_IRWXU */
#include <unistd.h>   /* access */
#include <fcntl.h>    /* open */
#include <stdarg.h>  /* va_list, va_start, va_end */

#include "log.h"
#include "strl.h"
#include "util.h"

void *xmalloc(size_t n) {
    void *p;

    if (n == 0) {
        die("Empty memory allocation");
    }

    p = malloc(n);

    if (!p) {
        die("Failed to allocate memory");
    }

    return p;
}

int cntchr(const char *str, int chr) {
    const char *tmp = str;
    int i = 0;

    for (; tmp[i]; tmp[i] == chr ? i++ : *tmp++);

    return i;
}

int cntlines(FILE *file) {
    char *line = 0;
    int i = 0;

    while (getline(&line, &(size_t){0}, file) != -1) {
        if (line[0] != '#' && line[0] != '\n') {
           i++;
        }
    }
    rewind(file);
    free(line);

    return i;
}

int strsuf(const char *str, const char *suf, size_t suf_len) {
    if (!str || !suf || !suf_len) {
       return -1;
    }

    return strncmp(&str[strlen(str) - suf_len], suf, suf_len);
}

void xsnprintf(char *str, size_t size, const char *fmt, ...) {
    va_list va;
    unsigned int err;

    va_start(va, fmt);
    err = vsnprintf(str, size, fmt, va);
    va_end(va);

    if (err < 1) {
        die("snprintf failed to construct string");
    }

    if (err > size) {
        die("snprintf result exceeds buffer size");
    }
}

int exists_at(const char *path, const char *file, const int flags) {
    int dfd;
    int ffd;

    dfd = open(path, O_RDONLY | O_DIRECTORY);

    if (dfd == -1) {
        return 1;
    }

    ffd = openat(dfd, file, O_RDONLY | flags);

    close(dfd);

    if (ffd == -1) {
        return 1;
    }

    close(ffd);
    return 0;
}

FILE *fopenat(const char *path, const char *file, const char *mode) {
    int dfd;
    int ffd;

    dfd = open(path, O_RDONLY | O_DIRECTORY);

    if (dfd == -1) {
        return NULL;
    }

    ffd = openat(dfd, file, O_RDONLY);
    close(dfd);

    if (ffd == -1) {
        return NULL;
    }

    return fdopen(ffd, mode);
}