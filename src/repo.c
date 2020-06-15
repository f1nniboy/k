#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "repo.h"

char **repo_load(void) {
    char *path = getenv("KISS_PATH");

    if (path == NULL || path[0] == '\0') {
        printf("error: KISS_PATH must be set\n");
        exit(1);
    }

    // TODO: Append installed db.
    /* path = strjoin(path, "/var/db/kiss/installed", ":"); */

    return split_string(path, ":");
}