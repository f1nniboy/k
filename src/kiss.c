#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>

#include "util.h"
#include "vec.h"
#include "str.h"

#define DB_DIR "/var/db/kiss/installed"

enum actions {
    ACTION_ALTERNATIVES,
    ACTION_BUILD,
    ACTION_CHECKSUM,
    ACTION_DOWNLOAD,
    ACTION_INSTALL,
    ACTION_LIST,
    ACTION_REMOVE,
    ACTION_SEARCH,
    ACTION_UPDATE,
};

typedef struct package {
    char *name;    
    char *path;
} package;

static char **REPOS = NULL;

static void repo_init(void) {
    char *p = NULL;
    char *path = xgetenv("KISS_PATH");

    for (char *tok = strtok_r(path, ":", &p);
         tok != NULL;
         tok = strtok_r(NULL, ":", &p)) {

        if (tok[0] != '/') {
            die("relative path found in KISS_PATH");
        }

        vec_add(REPOS, strdup(tok));
    }

    free(p);
    free(path);

    vec_add(REPOS, strdup(DB_DIR));
}

static void repo_destroy(void) {
    for (size_t i = 0; i < vec_size(REPOS); i++) {
        free(REPOS[i]);
    }
    vec_free(REPOS);
}

static char *pkg_find(const char *pattern, int all) {
    glob_t buf;

    for (size_t i = 0; i < vec_size(REPOS); ++i) {
        str query = {0};

        str_cat(&query, REPOS[i]);
        str_cat(&query, "/");
        str_cat(&query, pattern);
        str_cat(&query, "/");

        glob(query.buf, i ? GLOB_APPEND : 0, NULL, &buf);

        str_free(&query);
    }

    char *match = NULL;

    if (buf.gl_pathc != 0 && buf.gl_pathv[0]) {
        match = strdup(buf.gl_pathv[0]);

        if (all) {
            for (size_t i = 0; i < buf.gl_pathc; i++) {
                puts(buf.gl_pathv[i]);
            }
        }
    }

    globfree(&buf);
    return match;
}

static int pkg_list(const char *name, int print) {
    str p = {0};

    str_cat(&p, DB_DIR);
    str_cat(&p, "/");
    str_cat(&p, name);

    int ret = is_dir(p.buf);

    if (ret == 0 && print) {
        puts(name);
    }

    str_free(&p);

    return ret;
}

static void run_extension(char *argv[]) {
    str cmd = {0};

    str_cat(&cmd, "kiss-");
    str_cat(&cmd, argv[1]);

    int err = execvp(cmd.buf, ++argv);

    str_free(&cmd);

    if (err == -1) {
        perror("execvp");
        exit(1);
    }
}

int main (int argc, char *argv[]) {
    int action = 0;

    if (argc < 2 || !argv[1] || !argv[1][0] || argv[1][0] == '-') {
        puts("kiss [b|c|d|l|s|v] [pkg]...");
        puts("alternatives List and swap to alternatives"); 
        puts("build        Build a package");
        puts("checksum     Generate checksums");
        puts("download     Pre-download all sources");
        puts("install      Install a package");
        puts("list         List installed packages");
        puts("remove       Remove a package");
        puts("search       Search for a package");
        puts("update       Update the system");
        puts("version:      Package manager version");
        puts("\nRun 'kiss help-ext' to see all actions");
        return 0;

    } else if (strcmp(argv[1], "build") == 0 ||
               strcmp(argv[1], "b") == 0) {
        action = ACTION_BUILD;

    } else if (strcmp(argv[1], "checksum") == 0 ||
               strcmp(argv[1], "c") == 0) {
        action = ACTION_CHECKSUM;

    } else if (strcmp(argv[1], "download") == 0 ||
               strcmp(argv[1], "d") == 0) {
        action = ACTION_DOWNLOAD;

    } else if (strcmp(argv[1], "install") == 0 ||
               strcmp(argv[1], "i") == 0) {
        action = ACTION_INSTALL;

    } else if (strcmp(argv[1], "list") == 0 ||
               strcmp(argv[1], "l") == 0) {
        action = ACTION_LIST;

    } else if (strcmp(argv[1], "remove") == 0 ||
               strcmp(argv[1], "r") == 0) {
        action = ACTION_REMOVE;

    } else if (strcmp(argv[1], "search") == 0 ||
               strcmp(argv[1], "s") == 0) {
        action = ACTION_SEARCH;

    } else if (strcmp(argv[1], "version") == 0 ||
               strcmp(argv[1], "v") == 0) {
        msg("0.0.1");
        return 0;

    } else {
        run_extension(argv);
        return 0;
    }

    repo_init();

    switch (action) {
        case ACTION_LIST: {
            for (int i = 2; i < argc; i++) {
                if (pkg_list(argv[i], 1)) {
                    die("package '%s' not installed", argv[i]);
                }
            }
            break;
        }

        case ACTION_SEARCH: {
            for (int i = 2; i < argc; i++) {
                char *match = pkg_find(argv[i], 1);

                if (match) {
                    free(match);
                } else {
                    die("no results for '%s'", argv[i]);
                }
            }

            break;
        }
    }

    repo_destroy();
}
