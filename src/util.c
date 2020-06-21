#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>   /* malloc, size_t */
#include <stdio.h>    /* printf */
#include <limits.h>   /* PATH_MAX  */
#include <string.h>   /* strncpy */
#include <sys/stat.h> /* mkdir */
#include <errno.h>    /* errno, EEXIST, S_IRWXU */
#include <unistd.h>   /* access */

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

    return i;
}

int strsuf(const char *str, const char *suf, size_t str_len, size_t suf_len) {
    if (!str || !suf || suf_len > str_len) {
       return 0;
    }

    return !strncmp(str + str_len - suf_len, suf, suf_len);
}

void mkdir_p(const char *dir) {
    char tmp[PATH_MAX];
    int err;
    char *p = 0;

    if (!dir) {
        die("mkdir input empty");
    }

    if (access(dir, F_OK) != -1) {
        return;
    }

    err = strlcpy(tmp, dir, PATH_MAX);

    if (err > PATH_MAX) {
        die("strlcpy truncated PATH");
    }

    for (p = tmp + 1; *p; p++) {
       if (*p == '/') {
           *p = 0;

           err = mkdir(tmp, S_IRWXU);

           if (err == -1 && errno != EEXIST) {
               die("Failed to create directory %s", tmp);
           }

           *p = '/';
       }
    }

    err = mkdir(tmp, S_IRWXU);

    if (err == -1 && errno != EEXIST) {
        die("Failed to create directory %s", tmp);
    }
}
