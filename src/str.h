#ifndef KISS_STR_H_
#define KISS_STR_H_

#include <stdio.h>
#include <sys/types.h>

typedef struct str {
    size_t len;
    size_t cap;
    char *buf;
} str;

#define str_free(s)      \
    do {                 \
        free((*s)->buf); \
        free((*s));      \
    } while (0)

#define str_undo(s, d)                             \
    do {                                           \
        if (d && d[0]) {                           \
            (*s)->buf[(*s)->len -= strlen(d)] = 0; \
        }                                          \
    } while (0)

#define str_alloc(s, l)                           \
    do {                                          \
        if (!(*s)) {                              \
            (*s) = calloc(1, sizeof(str));        \
                                                  \
            if (!(*s)) {                          \
                perror("calloc");                 \
                exit(1);                          \
            }                                     \
        }                                         \
                                                  \
        str *_p = *s;                             \
                                                  \
        if ((_p->len + l) >= _p->cap) {           \
            _p->cap += l;                         \
            _p->buf  = realloc(_p->buf, _p->cap); \
                                                  \
            if (!_p->buf) {                       \
                perror("realloc");                \
                exit(1);                          \
            }                                     \
        }                                         \
    } while (0)

#define str_push(s, d)                                \
    do {                                              \
        if (d && d[0]) {                              \
            size_t _l = strlen(d);                    \
            str_alloc(s, _l * 2);                     \
            memcpy((*s)->buf + (*s)->len, d, _l + 1); \
            (*s)->buf[(*s)->len += _l] = 0;           \
        }                                             \
    } while (0)

#define str_from(s, f, t)                            \
    do {                                             \
        int _l2 = snprintf(NULL, 0, f, t);           \
                                                     \
        if (_l2 > 0) {                               \
            str_alloc(s, (size_t) _l2);              \
                                                     \
            int e = snprintf(*s->buf + *s->len,      \
                            (size_t) _l2 + 1, f, t); \
                                                     \
            if (e != _l2) {                          \
                *s->buf[*s->len] = 0;                \
            } else {                                 \
                *s->len += (size_t) _l2;             \
            }                                        \
        }                                            \
    } while (0)

#endif
